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

    // �±״� ������Ʈ ��Ģ�� �°� �����ϼ���
    FGameplayTagContainer AssetTags;
    AssetTags.AddTag(MonsterTags::Ability_Boss_PhaseStart);
    // ����: ���� �ĺ� �±׵� ������ �߰�
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
        // Ÿ�̸� �̰��� ���� ����
        if (AActor* Boss = GetAvatarActorFromActorInfo())
        {
            Boss->GetWorldTimerManager().ClearTimer(SmashTimerHandle);
        }
        // ���� ���Žð� ������ �ٷ� ����
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

        // End ����: �׻� üũ
        if (Ratio <= EndHpRatioThreshold && !bShouldEndAfterCurrentSmash && !bEnding)
        {
            bShouldEndAfterCurrentSmash = true;

            if (AActor* Boss = GetAvatarActorFromActorInfo())
            {
                Boss->GetWorldTimerManager().ClearTimer(SmashTimerHandle); // �� ���Ž� ����
            }

            if (!bSmashInProgress)
            {
                BeginEndSequence(); // ���Ž� ���� �ƴϸ� �ٷ� ����
                return;
            }
            // ���Ž� ���̸� OnSmashMontageFinished���� ���� ����
        }

        // Start ����: ���� Phase ���� �� ���� ����
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

    // ���� ���� ����/���Ž� ��Ÿ�ְ� �ִٸ� �����ϰ� ����
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
        // ĳ����/��ȯ ������ ���� �� ���� �������
        Boss->SetBossState_EBB((uint8)EBossState_BB::Phase2_Attack);
    }

    Super::EndAbility(Handle, Info, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_BossPhase2::SmashTick()
{
    if (bShouldEndAfterCurrentSmash || bEnding)
    {
        // ���� ���Žø� �������� �ʰ� ��������
        BeginEndSequence();
        return;
    }


    if (bSmashInProgress) return; // ���� ���� ���� ���� ���̸� ��ŵ
    bSmashInProgress = true;

    ACharacter* C = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!C || !GroundSmashMontage) { bSmashInProgress = false; return; }

    // ����(��Ʈ������� �����ϸ� Launch ����)
    if (JumpZ > 0.f) C->LaunchCharacter(FVector(0, 0, JumpZ), false, true);

    // ��Ÿ��: Start��Loop(����) ����
    if (UAnimInstance* Anim = C->GetMesh()->GetAnimInstance())
    {
        Anim->Montage_JumpToSection(SEC_Start, GroundSmashMontage);
        Anim->Montage_SetNextSection(SEC_Start, SEC_Loop, GroundSmashMontage);
        Anim->Montage_SetNextSection(SEC_Loop, SEC_Loop, GroundSmashMontage);
    }

    // ��� & ���� ���
    if (auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, GroundSmashMontage, 1.f, NAME_None, false))
    {
        Task->OnCompleted.AddDynamic(this, &UGA_BossPhase2::OnSmashMontageFinished);
        Task->OnBlendOut.AddDynamic(this, &UGA_BossPhase2::OnSmashMontageFinished);
        Task->OnInterrupted.AddDynamic(this, &UGA_BossPhase2::OnSmashMontageFinished);
        Task->OnCancelled.AddDynamic(this, &UGA_BossPhase2::OnSmashMontageFinished);
        Task->ReadyForActivation();
    }

    // ���� �̺�Ʈ ��� (BossCharacter::Landed���� SendGameplayEvent)
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

    // ����� ����
    if (ShockwaveActorClass)
    {
        FTransform T(C->GetActorRotation(), C->GetActorLocation());
        AActor* Spawned = C->GetWorld()->SpawnActor<AActor>(ShockwaveActorClass, T);

        // ���� �ν�Ƽ������ ����(���� ���ؽ�Ʈ��)
        if (AShockwaveActor* SW = Cast<AShockwaveActor>(Spawned))
        {
            SW->SetInstigatorActor(C);
        }
    }
    // �Ǵ� ���⼭ �÷��̾�� ���� ������/GE ���� ����
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

    // 1) Hit GA ���
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        FGameplayTagContainer CancelTags;
        CancelTags.AddTag(MonsterTags::Ability_HitReact);
        ASC->CancelAbilities(&CancelTags);
    }

    // 2) ���� ���� ��Ÿ�� ����(����)
    if (ACharacter* C = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
    {
        if (UAnimInstance* Anim = C->GetMesh() ? C->GetMesh()->GetAnimInstance() : nullptr)
        {
            Anim->StopAllMontages(0.10f);
        }
    }

    // 3) ���� ����(����)
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

    // 4) ���� ƽ���� Start ��Ÿ�� ó��
    if (AActor* Boss = GetAvatarActorFromActorInfo())
    {
        Boss->GetWorldTimerManager().SetTimerForNextTick(this, &UGA_BossPhase2::PlayStartMontageThenStartSmash);
    }
}

void UGA_BossPhase2::PlayStartMontageThenStartSmash()
{
    // StartMontage �̼����̸� �ٷ� ���� ����
    if (!StartMontage)
    {
        SpawnWeakPoints(); // <- ���� ����
        StartSmashLoop();
        return;
    }

    if (auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, StartMontage, 1.f, NAME_None, /*bStopWhenAbilityEnds=*/false))
    {
        // � ���� ���̽��� Smash ������ ����
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
        // ������
        SpawnWeakPoints();
        StartSmashLoop();
    }
}

void UGA_BossPhase2::StartSmashLoop()
{
    if (bShouldEndAfterCurrentSmash || bEnding)  // End ��û �� ���� ���� ����
    {
        BeginEndSequence();
        return;
    }


    // �̹� Ÿ�̸Ӱ� �ִٸ� �ߺ� ����
    if (AActor* Boss = GetAvatarActorFromActorInfo())
    {
        if (!Boss->GetWorldTimerManager().IsTimerActive(SmashTimerHandle))
        {
            Boss->GetWorldTimerManager().SetTimer(
                SmashTimerHandle, this, &UGA_BossPhase2::SmashTick,
                SmashInterval, true, 1.f /*�ʱ� ����*/);
        }
    }
}

void UGA_BossPhase2::BeginEndSequence()
{
    if (bEnding) return;
    bEnding = true;

    // � ���� ���Ž� ���� ����
    if (AActor* Boss = GetAvatarActorFromActorInfo())
    {
        Boss->GetWorldTimerManager().ClearTimer(SmashTimerHandle);
    }

    // ���Žð� ���� ���̸�(������) ���⼭ ������ ��ٸ����� return ó���� ����
    if (bSmashInProgress)
    {
        // ���������� ��ٸ��� �ʹٸ� �ܼ� return; �ص� ��.
        // ������ ���⼭�� ���Ž� ���� �ݹ鿡���� �������� ������ ���������Ƿ� ��� false.
    }

    // �ǰ� �� ���(���� ���� ��� ����)
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        FGameplayTagContainer CancelTags;
        CancelTags.AddTag(MonsterTags::Ability_HitReact);
        ASC->CancelAbilities(&CancelTags);
    }

    // ���� ���� ���Ž�/��Ÿ ��Ÿ�� ����(����)
    if (ACharacter* C = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
    {
        if (UAnimInstance* Anim = C->GetMesh() ? C->GetMesh()->GetAnimInstance() : nullptr)
        {
            Anim->StopAllMontages(0.10f);
        }
    }

    // ���� ��Ÿ�� ��� �� �Ϸ�Ǹ� EndAbility
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

    // ���� ��Ÿ�ְ� ���ų� Task ���� �� ������
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

        // ���� �� ����(����)
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

    // �ı� �̺�Ʈ ���� ���(���� ���� �� �� �����Ƿ� WaitGameplayEvent�� ���� ����)
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
        Damage = Payload.EventMagnitude; // ���ͺ� Ŀ���� ���� ���
    }

    if (GE_WeakPointDamageToBoss)
    {
        FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
        // Instigator�� ���� ���ͷ� ����(����)
        // Instigator: ����(�ڱ� �ڽ�)�� �δ� ���� ����
        AActor* BossActor = GetAvatarActorFromActorInfo();

        // EffectCauser: ���� ����(�̺�Ʈ ���� ��)�� �ְ� ������ const_cast
        AActor* EffectCauser = nullptr;
        if (Payload.Instigator)
        {
            EffectCauser = const_cast<AActor*>(Payload.Instigator.Get());
        }

        // ������ ���(Self-apply)�̹Ƿ� �̷��� ����
        Ctx.AddInstigator(BossActor, EffectCauser);

        FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(GE_WeakPointDamageToBoss, 1.f, Ctx);
        if (Spec.IsValid())
        {
            Spec.Data->SetSetByCallerMagnitude(MonsterTags::Data_Damage, Damage);
            ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
        }
    }
}
