#include "Boss/GA_BossPhase2.h"
#include "AbilitySystemComponent.h"
#include "MonsterAttributeSet.h"
#include "MonsterTags.h"
#include "GameFramework/Actor.h"
#include "Boss/BossCharacter.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Data/staticName.h"
#include "Boss/ShockwaveActor.h"
#include "Kismet/GameplayStatics.h"           
#include "Components/SkeletalMeshComponent.h" 
#include "Boss/WeakPointActor.h"
#include "NavigationSystem.h"
static FVector RandomOnRing2D(const FVector& _Center, float _Radius)
{
    float Theta = FMath::FRandRange(0.f, 2.f * PI);
    return _Center + FVector(_Radius * FMath::Cos(Theta), _Radius * FMath::Sin(Theta), 0.f);
}

UGA_BossPhase2::UGA_BossPhase2()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    // 태그는 프로젝트 규칙에 맞게 조정하세요
    FGameplayTagContainer AssetTags;
    AssetTags.AddTag(MonsterTags::Ability_Boss_PhaseStart);
    // 선택: 개별 식별 태그도 있으면 추가
    // AssetTags.AddTag(MonsterTags::Ability_Boss_Phase2);
    SetAssetTags(AssetTags);

    ActivationBlockedTags.AddTag(MonsterTags::State_Dying);
    ActivationBlockedTags.AddTag(MonsterTags::State_Dead);
}

void UGA_BossPhase2::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, Info, ActivationInfo, TriggerEventData);

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (!ASC)
    {
        EndAbility(Handle, Info, ActivationInfo, true, false);
        return;
    }

    const float CurHP = ASC->GetNumericAttribute(UMonsterAttributeSet::GetHealthAttribute());
    const float MaxHP = ASC->GetNumericAttribute(UMonsterAttributeSet::GetMaxHealthAttribute());
    const float Ratio = (MaxHP > 0.f) ? (CurHP / MaxHP) : 0.f;

    if (Ratio <= EndHpRatioThreshold)
    {
        bShouldEndAfterCurrentSmash = true;
        // 타이머 미가동 상태 보장
        if (AActor* Boss = GetAvatarActorFromActorInfo())
        {
            Boss->GetWorldTimerManager().ClearTimer(SmashTimerHandle);
        }
        // 아직 스매시가 없으니 바로 엔딩
        BeginEndSequence();
        return;
    }

    if (Ratio <= StartHpRatioThreshold && !bPhaseStarted)
    {
        StartPhase();
        return;
    }

    BindHPThresholdWatch();
}

void UGA_BossPhase2::BindHPThresholdWatch()
{
    if (HPChangeHandle.IsValid()) return;

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        HPChangeHandle = ASC->GetGameplayAttributeValueChangeDelegate(
            UMonsterAttributeSet::GetHealthAttribute()
        ).AddUObject(this, &UGA_BossPhase2::OnHPChangedNative);
    }
}

void UGA_BossPhase2::UnbindHPThresholdWatch()
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

void UGA_BossPhase2::OnHPChangedNative(const FOnAttributeChangeData& Data)
{
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        const float Max = ASC->GetNumericAttribute(UMonsterAttributeSet::GetMaxHealthAttribute());
        if (Max <= 0.f) return;

        const float Ratio = Data.NewValue / Max;

        // End 조건: 항상 체크
        if (Ratio <= EndHpRatioThreshold && !bShouldEndAfterCurrentSmash && !bEnding)
        {
            bShouldEndAfterCurrentSmash = true;

            if (AActor* Boss = GetAvatarActorFromActorInfo())
            {
                Boss->GetWorldTimerManager().ClearTimer(SmashTimerHandle); // 새 스매시 방지
            }

            if (!bSmashInProgress)
            {
                BeginEndSequence(); // 스매시 중이 아니면 바로 엔딩
                return;
            }
            // 스매시 중이면 OnSmashMontageFinished에서 엔딩 진입
        }

        // Start 조건: 아직 Phase 시작 안 했을 때만
        if (!bPhaseStarted && Ratio <= StartHpRatioThreshold)
        {
            UnbindHPThresholdWatch();
            StartPhase();
        }
    }
}

void UGA_BossPhase2::StartPhase()
{
    if (bPhaseStarted) return;
    bPhaseStarted = true;

    if (ABossCharacter* BossCharacter = Cast<ABossCharacter>(GetAvatarActorFromActorInfo()))
    {
        BossCharacter->SetBossState_EBB((uint8)EBossState_BB::InPhase2);
    }
    BindHPThresholdWatch();

    BeginStartSequence();
}

void UGA_BossPhase2::EndAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    if (AActor* Boss = GetAvatarActorFromActorInfo())
    {
        Boss->GetWorldTimerManager().ClearTimer(SmashTimerHandle);
    }
    bSmashInProgress = false;
    bStartingPhase2 = false;

    // 진행 중인 시작/스매시 몽타주가 있다면 안전하게 중지
    if (ACharacter* C = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
    {
        if (UAnimInstance* Anim = C->GetMesh() ? C->GetMesh()->GetAnimInstance() : nullptr)
        {
            Anim->StopAllMontages(0.10f);
        }
    }


    UnbindHPThresholdWatch();

    if (ABossCharacter* Boss = Cast<ABossCharacter>(GetAvatarActorFromActorInfo()))
    {
        // 캐스팅/소환 페이즈 종료 → 공격 페이즈로
        Boss->SetBossState_EBB((uint8)EBossState_BB::Phase2_Attack);
    }

    Super::EndAbility(Handle, Info, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_BossPhase2::SmashTick()
{
    if (bShouldEndAfterCurrentSmash || bEnding)
    {
        // 현재 스매시를 시작하지 않고 엔딩으로
        BeginEndSequence();
        return;
    }


    if (bSmashInProgress) return; // 이전 쿵이 아직 진행 중이면 스킵
    bSmashInProgress = true;

    ACharacter* C = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!C || !GroundSmashMontage) { bSmashInProgress = false; return; }

    // 점프(루트모션으로 점프하면 Launch 생략)
    if (JumpZ > 0.f) C->LaunchCharacter(FVector(0, 0, JumpZ), false, true);

    // 몽타주: Start→Loop(무한) 설정
    if (UAnimInstance* Anim = C->GetMesh()->GetAnimInstance())
    {
        Anim->Montage_JumpToSection(SEC_Start, GroundSmashMontage);
        Anim->Montage_SetNextSection(SEC_Start, SEC_Loop, GroundSmashMontage);
        Anim->Montage_SetNextSection(SEC_Loop, SEC_Loop, GroundSmashMontage);
    }

    // 재생 & 종료 대기
    if (auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, GroundSmashMontage, 1.f, NAME_None, false))
    {
        Task->OnCompleted.AddDynamic(this, &UGA_BossPhase2::OnSmashMontageFinished);
        Task->OnBlendOut.AddDynamic(this, &UGA_BossPhase2::OnSmashMontageFinished);
        Task->OnInterrupted.AddDynamic(this, &UGA_BossPhase2::OnSmashMontageFinished);
        Task->OnCancelled.AddDynamic(this, &UGA_BossPhase2::OnSmashMontageFinished);
        Task->ReadyForActivation();
    }

    // 착지 이벤트 대기 (BossCharacter::Landed에서 SendGameplayEvent)
    if (auto* Wait = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, LandEventTag))
    {
        Wait->EventReceived.AddDynamic(this, &UGA_BossPhase2::OnLandEvent);
        Wait->ReadyForActivation();
    }
}

void UGA_BossPhase2::OnLandEvent(FGameplayEventData Payload)
{
    ACharacter* C = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (C && GroundSmashMontage)
    {
        if (UAnimInstance* Anim = C->GetMesh()->GetAnimInstance())
        {
            Anim->Montage_SetNextSection(SEC_Loop, SEC_End, GroundSmashMontage);
            Anim->Montage_JumpToSection(SEC_End, GroundSmashMontage);
        }
    }

    // 충격파 스폰
    if (ShockwaveActorClass)
    {
        FTransform T(C->GetActorRotation(), C->GetActorLocation());
        AActor* Spawned = C->GetWorld()->SpawnActor<AActor>(ShockwaveActorClass, T);

        // 보스 인스티게이터 전달(피해 컨텍스트용)
        if (AShockwaveActor* SW = Cast<AShockwaveActor>(Spawned))
        {
            SW->SetInstigatorActor(C);
        }
    }
    // 또는 여기서 플레이어에게 라디얼 데미지/GE 적용 로직
}

void UGA_BossPhase2::OnSmashMontageFinished()
{
    bSmashInProgress = false;
    
    if (bShouldEndAfterCurrentSmash && !bEnding)
    {
        BeginEndSequence();
        return;
    }
}

void UGA_BossPhase2::BeginStartSequence()
{
    if (bStartingPhase2) return;
    bStartingPhase2 = true;

    // 1) Hit GA 취소
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        FGameplayTagContainer CancelTags;
        CancelTags.AddTag(MonsterTags::Ability_HitReact);
        ASC->CancelAbilities(&CancelTags);
    }

    // 2) 진행 중인 몽타주 정지(안전)
    if (ACharacter* C = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
    {
        if (UAnimInstance* Anim = C->GetMesh() ? C->GetMesh()->GetAnimInstance() : nullptr)
        {
            Anim->StopAllMontages(0.10f);
        }
    }

    // 3) 시작 사운드(선택)
    if (StartSound)
    {
        if (ACharacter* Char = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
        {
            UGameplayStatics::SpawnSoundAttached(StartSound, Char->GetMesh());
        }
        else if (AActor* Boss = GetAvatarActorFromActorInfo())
        {
            UGameplayStatics::PlaySoundAtLocation(Boss, StartSound, Boss->GetActorLocation());
        }
    }

    // 4) 다음 틱에서 Start 몽타주 처리
    if (AActor* Boss = GetAvatarActorFromActorInfo())
    {
        Boss->GetWorldTimerManager().SetTimerForNextTick(this, &UGA_BossPhase2::PlayStartMontageThenStartSmash);
    }
}

void UGA_BossPhase2::PlayStartMontageThenStartSmash()
{
    // StartMontage 미설정이면 바로 루프 시작
    if (!StartMontage)
    {
        SpawnWeakPoints(); // <- 약점 스폰
        StartSmashLoop();
        return;
    }

    if (auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, StartMontage, 1.f, NAME_None, /*bStopWhenAbilityEnds=*/false))
    {
        // 어떤 종료 케이스든 Smash 루프로 진입
        Task->OnCompleted.AddDynamic(this, &UGA_BossPhase2::StartSmashLoop);
        Task->OnBlendOut.AddDynamic(this, &UGA_BossPhase2::StartSmashLoop);
        Task->OnInterrupted.AddDynamic(this, &UGA_BossPhase2::StartSmashLoop);
        Task->OnCancelled.AddDynamic(this, &UGA_BossPhase2::StartSmashLoop);
        Task->ReadyForActivation();
        SpawnWeakPoints();
        return;
    }
    else
    {
        // 안전망
        SpawnWeakPoints();
        StartSmashLoop();
    }
}

void UGA_BossPhase2::StartSmashLoop()
{
    if (bShouldEndAfterCurrentSmash || bEnding)  // End 요청 시 루프 시작 금지
    {
        BeginEndSequence();
        return;
    }


    // 이미 타이머가 있다면 중복 방지
    if (AActor* Boss = GetAvatarActorFromActorInfo())
    {
        if (!Boss->GetWorldTimerManager().IsTimerActive(SmashTimerHandle))
        {
            Boss->GetWorldTimerManager().SetTimer(
                SmashTimerHandle, this, &UGA_BossPhase2::SmashTick,
                SmashInterval, true, 1.f /*초기 지연*/);
        }
    }
}

void UGA_BossPhase2::BeginEndSequence()
{
    if (bEnding) return;
    bEnding = true;

    // 어떤 경우든 스매시 루프 정지
    if (AActor* Boss = GetAvatarActorFromActorInfo())
    {
        Boss->GetWorldTimerManager().ClearTimer(SmashTimerHandle);
    }

    // 스매시가 진행 중이면(안전망) 여기서 끝까지 기다리려면 return 처리도 가능
    if (bSmashInProgress)
    {
        // 보수적으로 기다리고 싶다면 단순 return; 해도 됨.
        // 하지만 여기서는 스매시 종료 콜백에서만 들어오도록 위에서 통제했으므로 통상 false.
    }

    // 피격 등 취소(엔딩 방해 요소 제거)
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        FGameplayTagContainer CancelTags;
        CancelTags.AddTag(MonsterTags::Ability_HitReact);
        ASC->CancelAbilities(&CancelTags);
    }

    // 진행 중인 스매시/기타 몽타주 정지(안전)
    if (ACharacter* C = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
    {
        if (UAnimInstance* Anim = C->GetMesh() ? C->GetMesh()->GetAnimInstance() : nullptr)
        {
            Anim->StopAllMontages(0.10f);
        }
    }

    // 엔딩 몽타주 재생 → 완료되면 EndAbility
    PlayEndMontageAndFinish();
}

void UGA_BossPhase2::PlayEndMontageAndFinish()
{
    if (EndMontage)
    {
        if (auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, NAME_None, EndMontage, 1.f, NAME_None, /*bStopWhenAbilityEnds=*/false))
        {
            Task->OnCompleted.AddDynamic(this, &UGA_BossPhase2::K2_EndAbility);
            Task->OnBlendOut.AddDynamic(this, &UGA_BossPhase2::K2_EndAbility);
            Task->OnInterrupted.AddDynamic(this, &UGA_BossPhase2::K2_EndAbility);
            Task->OnCancelled.AddDynamic(this, &UGA_BossPhase2::K2_EndAbility);
            Task->ReadyForActivation();
            return;
        }
    }

    // 엔딩 몽타주가 없거나 Task 실패 시 안전망
    K2_EndAbility();
}

void UGA_BossPhase2::SpawnWeakPoints()
{
    if (!WeakPointClass) return;

    AActor* Boss = GetAvatarActorFromActorInfo();
    if (!Boss) return;

    UWorld* World = Boss->GetWorld();
    if (!World) return;

    UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(World);
    FVector Origin = Boss->GetActorLocation();

    for (int32 i = 0; i < WeakPointSpawnCount; ++i)
    {
        FVector Desired = RandomOnRing2D(Origin, WeakPointSpawnRadius);

        // 내비 위 보정(선택)
        if (Nav)
        {
            FNavLocation Out;
            if (Nav->ProjectPointToNavigation(Desired, Out, FVector(200.f)))
            {
                Desired = Out.Location;
            }
        }

        FTransform T(FRotator::ZeroRotator, Desired);
        AActor* Spawned = World->SpawnActor<AActor>(WeakPointClass, T);

        if (AWeakPointActor* WP = Cast<AWeakPointActor>(Spawned))
        {
            WP->InitializeWeakPoint(Boss, WeakPointDamageToBoss);
        }
    }

    // 파괴 이벤트 수신 대기(여러 개가 올 수 있으므로 WaitGameplayEvent는 재사용 안전)
    if (UAbilityTask_WaitGameplayEvent* Wait =
        UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, MonsterTags::Event_Boss_P2_WeakPointDestroyed, nullptr, false, true))
    {
        Wait->EventReceived.AddDynamic(this, &UGA_BossPhase2::OnWeakPointDestroyedEvent);
        Wait->ReadyForActivation();
    }
}

void UGA_BossPhase2::OnWeakPointDestroyedEvent(FGameplayEventData Payload)
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (!ASC) return;

    float Damage = WeakPointDamageToBoss;
    if (Payload.EventMagnitude > 0.f)
    {
        Damage = Payload.EventMagnitude; // 액터별 커스텀 피해 허용
    }

    if (GE_WeakPointDamageToBoss)
    {
        FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
        // Instigator를 약점 액터로 세팅(선택)
        // Instigator: 보스(자기 자신)로 두는 편이 안전
        AActor* BossActor = GetAvatarActorFromActorInfo();

        // EffectCauser: 약점 액터(이벤트 보낸 쪽)를 넣고 싶으면 const_cast
        AActor* EffectCauser = nullptr;
        if (Payload.Instigator)
        {
            EffectCauser = const_cast<AActor*>(Payload.Instigator.Get());
        }

        // 보스가 대상(Self-apply)이므로 이렇게 세팅
        Ctx.AddInstigator(BossActor, EffectCauser);

        FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(GE_WeakPointDamageToBoss, 1.f, Ctx);
        if (Spec.IsValid())
        {
            Spec.Data->SetSetByCallerMagnitude(MonsterTags::Data_Damage, Damage);
            ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
        }
    }
}
