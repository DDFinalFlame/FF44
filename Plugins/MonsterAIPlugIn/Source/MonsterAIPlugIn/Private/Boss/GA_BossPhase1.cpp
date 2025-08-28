
#include "Boss/GA_BossPhase1.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "TimerManager.h"

#include "MonsterTags.h"
#include "MonsterAttributeSet.h"
#include "Monster/MonsterCharacter.h"
#include "Boss/BossCharacter.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"


static const FName KEY_TargetActor(TEXT("TargetActor"));
static const FName KEY_MonsterState(TEXT("MonsterState"));

UGA_BossPhase1::UGA_BossPhase1()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    FGameplayTagContainer AssetTags;
    AssetTags.AddTag(MonsterTags::Ability_Boss_PhaseStart);
    SetAssetTags(AssetTags);

    ActivationBlockedTags.AddTag(MonsterTags::State_Dying);
    ActivationBlockedTags.AddTag(MonsterTags::State_Dead);
}

void UGA_BossPhase1::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, Info, ActivationInfo, TriggerEventData);

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    AActor* Boss = GetAvatarActorFromActorInfo();
    if (!ASC || !Boss)
    {
        EndAbility(Handle, Info, ActivationInfo, true, false);
        return;
    }

    const float CurHP = ASC->GetNumericAttribute(UMonsterAttributeSet::GetHealthAttribute());
    const float MaxHP = ASC->GetNumericAttribute(UMonsterAttributeSet::GetMaxHealthAttribute());
    const float Ratio = (MaxHP > 0.f) ? (CurHP / MaxHP) : 0.f;

    if (Ratio <= StartHpRatioThreshold && !bPhaseStarted)
    {
        StartPhase();
        return;
    }

    // 아직 임계 위 → HP 변화 감시 바인딩
    BindHPThresholdWatch();
}

void UGA_BossPhase1::BindHPThresholdWatch()
{
    if (HPChangeHandle.IsValid()) return;

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        HPChangeHandle = ASC->GetGameplayAttributeValueChangeDelegate(
            UMonsterAttributeSet::GetHealthAttribute()
        ).AddUObject(this, &UGA_BossPhase1::OnHPChangedNative);
    }
}

void UGA_BossPhase1::UnbindHPThresholdWatch()
{
    if (!HPChangeHandle.IsValid()) return;

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        ASC->GetGameplayAttributeValueChangeDelegate(
            UMonsterAttributeSet::GetHealthAttribute()
        ).Remove(HPChangeHandle);
    }
    HPChangeHandle.Reset();
}

void UGA_BossPhase1::OnHPChangedNative(const FOnAttributeChangeData& Data)
{
    if (bPhaseStarted) { UnbindHPThresholdWatch(); return; }

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        const float Max = ASC->GetNumericAttribute(UMonsterAttributeSet::GetMaxHealthAttribute());
        if (Max <= 0.f) return;

        const float Ratio = Data.NewValue / Max;
        if (Ratio <= StartHpRatioThreshold)
        {
            UnbindHPThresholdWatch();
            StartPhase();
        }
    }
}

void UGA_BossPhase1::StartPhase()
{
    if (bPhaseStarted) return;
    bPhaseStarted = true;

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    AActor* Boss = GetAvatarActorFromActorInfo();
    if (!ASC || !Boss)
    {
        K2_EndAbility();
        return;
    }

    // A) 무적 GE 적용 (쉴드 GCN 자동 실행)
    if (GE_BossInvuln)
    {
        FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
        Ctx.AddInstigator(Boss, Boss->GetInstigatorController());
        FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(GE_BossInvuln, 1.f, Ctx);
        InvulnHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
    }

    // B) 시작 연출: 몽타주 + 사운드
    if (StartMontage)
    {
        if (auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, NAME_None, StartMontage, 1.f, NAME_None, false))
        {
            Task->ReadyForActivation();
        }
    }
    if (StartSound)
    {
        if (ACharacter* Char = Cast<ACharacter>(Boss))
        {
            UGameplayStatics::SpawnSoundAttached(StartSound, Char->GetMesh());
        }
        else
        {
            UGameplayStatics::PlaySoundAtLocation(Boss, StartSound, Boss->GetActorLocation());
        }
    }

    // C) 소환
    AliveMinionCount = SpawnMinions(Boss);

    // D) 소환몹 사망 이벤트 대기 (미니언 Death GA에서 Event.Minion.Died 송신)
    if (UAbilityTask_WaitGameplayEvent* WaitEvt =
        UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, MonsterTags::Event_Minion_Died))
    {
        WaitEvt->EventReceived.AddDynamic(this, &UGA_BossPhase1::OnMinionDiedEvent);
        WaitEvt->ReadyForActivation();
    }

    if (ABossCharacter* BossCharacter = Cast<ABossCharacter>(GetAvatarActorFromActorInfo()))
    {
        BossCharacter->SetBossState_EBB((uint8)EBossState_BB::InPhase1);
    }


    // E) 캐스팅 루프 시작
    StartCastTick();
}

AActor* UGA_BossPhase1::GetPhaseTargetPlayer() const
{
    // 필요에 따라 플레이어 인덱스/타겟 규칙 변경
    return UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
}

// ----- 미니언 BB 세팅 시도 (성공 시 true) -----
bool UGA_BossPhase1::TrySetupMinionBlackboard(AMonsterCharacter* MC, AActor* Player)
{
    if (!MC) return false;

    // 컨트롤러 보장
    AAIController* AICon = Cast<AAIController>(MC->GetController());
    if (!AICon)
    {
        MC->SpawnDefaultController();
        AICon = Cast<AAIController>(MC->GetController());
        if (!AICon) return false; // 아직 컨트롤러가 없음 → 재시도
    }

    UBlackboardComponent* BB = AICon->GetBlackboardComponent();
    if (!BB) return false; // BB 아직 초기화 전 → 재시도

    // 값 세팅
    BB->SetValueAsObject(KEY_TargetActor, Player);
    BB->SetValueAsEnum(KEY_MonsterState, DesiredMinionState);


    return true;
}

// ----- 대기열에 추가 -----
void UGA_BossPhase1::EnqueueMinionInit(AMonsterCharacter* MC)
{
    if (!MC) return;
    PendingInitMinions.AddUnique(MC);

    // 타이머가 돌고 있지 않으면 시작
    AActor* Boss = GetAvatarActorFromActorInfo();
    if (Boss && !Boss->GetWorldTimerManager().IsTimerActive(MinionInitTimerHandle))
    {
        Boss->GetWorldTimerManager().SetTimer(
            MinionInitTimerHandle, this, &UGA_BossPhase1::ProcessPendingMinionInits, 0.1f, true, 0.1f);
    }
}

// ----- 대기열 처리 -----
void UGA_BossPhase1::ProcessPendingMinionInits()
{
    AActor* Player = GetPhaseTargetPlayer();
    if (!Player)
    {
        // 플레이어 없으면 다음 틱에 다시
        return;
    }

    // 뒤에서부터 제거
    for (int32 i = PendingInitMinions.Num() - 1; i >= 0; --i)
    {
        TWeakObjectPtr<AMonsterCharacter> WeakMC = PendingInitMinions[i];
        AMonsterCharacter* MC = WeakMC.Get();
        if (!MC)
        {
            PendingInitMinions.RemoveAtSwap(i);
            continue;
        }

        if (TrySetupMinionBlackboard(MC, Player))
        {
            PendingInitMinions.RemoveAtSwap(i);
        }
    }

    // 모두 처리 끝나면 타이머 정지
    if (PendingInitMinions.Num() == 0)
    {
        if (AActor* Boss = GetAvatarActorFromActorInfo())
        {
            Boss->GetWorldTimerManager().ClearTimer(MinionInitTimerHandle);
        }
    }
}


int32 UGA_BossPhase1::SpawnMinions(AActor* Boss)
{
    if (!Boss || !MinionClass) return 0;

    UWorld* World = Boss->GetWorld();
    if (!World) return 0;

    const int32 Count = FMath::Max(1, FMath::RandRange(SpawnCountMin, SpawnCountMax));
    const FVector Origin = Boss->GetActorLocation();
    const float StepDeg = 360.f / Count;

    UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(World);
    AActor* Player = GetPhaseTargetPlayer();

    for (int32 i = 0; i < Count; ++i)
    {
        const float Deg = StepDeg * i;
        const FVector Dir = UKismetMathLibrary::GetForwardVector(FRotator(0.f, Deg, 0.f));
        const FVector Desired = Origin + Dir * SpawnRadius;

        FVector SpawnLoc = Desired;
        if (Nav)
        {
            FNavLocation Out;
            if (Nav->ProjectPointToNavigation(Desired, Out, FVector(200.f)))
            {
                SpawnLoc = Out.Location;
            }
        }

        const FTransform T(FRotator::ZeroRotator, SpawnLoc);
        AActor* Spawned = World->SpawnActor<AActor>(MinionClass, T);
        if (AMonsterCharacter* MC = Cast<AMonsterCharacter>(Spawned))
        {
            MC->SetOwnerBoss(GetAvatarActorFromActorInfo()); 

            if (!Player || !TrySetupMinionBlackboard(MC, Player))
            {
                EnqueueMinionInit(MC);
            }
        }
    }
    return Count;
}

void UGA_BossPhase1::StartCastTick()
{
    AActor* Boss = GetAvatarActorFromActorInfo();
    if (!Boss || CastInterval <= 0.f) return;

    Boss->GetWorldTimerManager().SetTimer(
        CastTimerHandle,
        [this]() { DoCastOnce(); },
        CastInterval,
        true,
        CastInterval * 0.5f // 초기 지연
    );
}

void UGA_BossPhase1::DoCastOnce()
{
    if (CastLoopMontage)
    {
        if (auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, NAME_None, CastLoopMontage, 1.f, NAME_None, false))
        {
            Task->ReadyForActivation();
        }
    }

    // TODO: 여기서 투사체/광역마법 등 실제 캐스팅 로직 호출
}

void UGA_BossPhase1::OnMinionDiedEvent(FGameplayEventData Payload)
{
    if (AliveMinionCount > 0) --AliveMinionCount;

    if (AliveMinionCount == 0)
    {
        if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
        {
            if (InvulnHandle.IsValid())
            {
                ASC->RemoveActiveGameplayEffect(InvulnHandle); // 무적 해제(쉴드 GCN 자동 Off)
                InvulnHandle.Invalidate();
            }
        }
        K2_EndAbility();
    }
}

void UGA_BossPhase1::EndAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    if (AActor* Boss = GetAvatarActorFromActorInfo())
    {
        Boss->GetWorldTimerManager().ClearTimer(CastTimerHandle);
    }

    UnbindHPThresholdWatch();

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        if (InvulnHandle.IsValid())
        {
            ASC->RemoveActiveGameplayEffect(InvulnHandle);
            InvulnHandle.Invalidate();
        }
        ASC->RemoveGameplayCue(MonsterTags::GC_Boss_InvulnShield);
    }
    
    if (ABossCharacter* Boss = Cast<ABossCharacter>(GetAvatarActorFromActorInfo()))
    {
        // 캐스팅/소환 페이즈 종료 → 공격 페이즈로
        Boss->SetBossState_EBB((uint8)EBossState_BB::Phase1_Attack);
    }

    Super::EndAbility(Handle, Info, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}