
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

    // ���� �Ӱ� �� �� HP ��ȭ ���� ���ε�
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

    // A) ���� GE ���� (���� GCN �ڵ� ����)
    if (GE_BossInvuln)
    {
        FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
        Ctx.AddInstigator(Boss, Boss->GetInstigatorController());
        FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(GE_BossInvuln, 1.f, Ctx);
        InvulnHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
    }

    // B) ���� ����: ��Ÿ�� + ����
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

    // C) ��ȯ
    AliveMinionCount = SpawnMinions(Boss);

    // D) ��ȯ�� ��� �̺�Ʈ ��� (�̴Ͼ� Death GA���� Event.Minion.Died �۽�)
    if (UAbilityTask_WaitGameplayEvent* WaitEvt =
        UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, MonsterTags::Event_Minion_Died))
    {
        WaitEvt->EventReceived.AddDynamic(this, &UGA_BossPhase1::OnMinionDiedEvent);
        WaitEvt->ReadyForActivation();
    }

    // E) ĳ���� ���� ����
    StartCastTick();
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
        }
        // �ʿ� ��: ���� ���� ����(�̴Ͼ��� Death �� Boss�� �̺�Ʈ �۽��� �� ���)
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
        CastInterval * 0.5f // �ʱ� ����
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

    // TODO: ���⼭ ����ü/�������� �� ���� ĳ���� ���� ȣ��
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
                ASC->RemoveActiveGameplayEffect(InvulnHandle); // ���� ����(���� GCN �ڵ� Off)
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
    


    Super::EndAbility(Handle, Info, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}