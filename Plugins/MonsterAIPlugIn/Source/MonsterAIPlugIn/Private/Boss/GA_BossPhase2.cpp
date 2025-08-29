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

#include "Kismet/GameplayStatics.h"           
#include "Components/SkeletalMeshComponent.h" 
//static const FName SEC_Start("Start");
//static const FName SEC_Loop("Loop");
//static const FName SEC_End("End");

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

void UGA_BossPhase2::StartPhase()
{
    if (bPhaseStarted) return;
    bPhaseStarted = true;

    if (ABossCharacter* BossCharacter = Cast<ABossCharacter>(GetAvatarActorFromActorInfo()))
    {
        BossCharacter->SetBossState_EBB((uint8)EBossState_BB::InPhase2);
    }

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
    Super::EndAbility(Handle, Info, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_BossPhase2::SmashTick()
{
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
        C->GetWorld()->SpawnActor<AActor>(ShockwaveActorClass, T);
    }
    // �Ǵ� ���⼭ �÷��̾�� ���� ������/GE ���� ����
}

void UGA_BossPhase2::OnSmashMontageFinished()
{
    bSmashInProgress = false;
    // ���⼭ �ƹ� �͵� �� �ص� Ÿ�̸Ӱ� ���� 2.5�ʿ� �ٽ� SmashTick�� ȣ��
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
    }
    else
    {
        // ������
        StartSmashLoop();
    }
}

void UGA_BossPhase2::StartSmashLoop()
{
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