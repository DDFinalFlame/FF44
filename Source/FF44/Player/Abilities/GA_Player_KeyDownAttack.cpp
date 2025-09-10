#include "GA_Player_KeyDownAttack.h"

#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameplayTagContainer.h"
#include "Kismet/GameplayStatics.h"

#include "Player/BasePlayer.h"
#include "Weapon/BaseWeapon.h"
#include "Player/Data/PlayerTags.h"

UGA_Player_KeyDownAttack::UGA_Player_KeyDownAttack()
{
    ActivationOwnedTags.AddTag(PlayerTags::State_Player_KeyDownAttack);
}

void UGA_Player_KeyDownAttack::CommitExecute(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    if (UAnimInstance* AnimInst = OwnerPlayer->GetMesh()->GetAnimInstance())
    {
        AnimInst->OnPlayMontageNotifyBegin.AddDynamic(this, &UGA_Player_KeyDownAttack::BeginNotify);
        AnimInst->OnPlayMontageNotifyEnd.AddDynamic(this, &UGA_Player_KeyDownAttack::EndNotify);

        UAbilityTask_PlayMontageAndWait* Task =
            UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
                this,
                NAME_None,
                AttackMontage,
                MontagePlayRate,
                NAME_None,
                false,
                1.0f
            );

        UWorld* World = OwnerPlayer->GetWorld();

        if (LoopSwingSound)
            Audio = UGameplayStatics::SpawnSound2D(World, LoopSwingSound);

        if (StartVoice)
            UGameplayStatics::PlaySound2D(World, StartVoice);

        Task->OnCompleted.AddDynamic(this, &UGA_Player_KeyDownAttack::K2_EndAbility);
        Task->OnBlendOut.AddDynamic(this, &UGA_Player_KeyDownAttack::K2_EndAbility);
        Task->OnInterrupted.AddDynamic(this, &UGA_Player_KeyDownAttack::K2_EndAbility);
        Task->OnCancelled.AddDynamic(this, &UGA_Player_KeyDownAttack::K2_EndAbility);
        Task->ReadyForActivation();
    }

    OwnerPlayer->OnKeyDownAttackEnd.AddDynamic(this, &UGA_Player_KeyDownAttack::EndAttack);
}

void UGA_Player_KeyDownAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    if (UAnimInstance* AnimInst = OwnerPlayer->GetMesh()->GetAnimInstance())
    {
        AnimInst->OnPlayMontageNotifyBegin.RemoveDynamic(this, &UGA_Player_KeyDownAttack::BeginNotify);
        AnimInst->OnPlayMontageNotifyEnd.RemoveDynamic(this, &UGA_Player_KeyDownAttack::EndNotify);
        OwnerPlayer->OnKeyDownAttackEnd.RemoveDynamic(this, &UGA_Player_KeyDownAttack::EndAttack);
    }

    if (Audio) {
        Audio->Stop();
        Audio->DestroyComponent();
        Audio = nullptr;
    }

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Player_KeyDownAttack::BeginNotify(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
    if(NotifyName== BeginAttackName)
        OwnerWeapon->GetWeaponCollision()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void UGA_Player_KeyDownAttack::EndNotify(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
    if (NotifyName == EndAttackName)
        OwnerWeapon->GetWeaponCollision()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UGA_Player_KeyDownAttack::EndAttack()
{
    UAbilityTask_PlayMontageAndWait* Task =
        UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this,
            NAME_None,
            AttackMontage,
            MontagePlayRate,
            EndSectionName,
            false,
            1.0f
        );

    if (Audio) {
        Audio->Stop();
        Audio->DestroyComponent();
        Audio = nullptr;
    }

    UWorld* World = OwnerPlayer->GetWorld();
    UGameplayStatics::PlaySound2D(World, EndSwingSound);

    Task->OnCompleted.AddDynamic(this, &UGA_Player_KeyDownAttack::K2_EndAbility);
    Task->OnBlendOut.AddDynamic(this, &UGA_Player_KeyDownAttack::K2_EndAbility);
    Task->OnInterrupted.AddDynamic(this, &UGA_Player_KeyDownAttack::K2_EndAbility);
    Task->OnCancelled.AddDynamic(this, &UGA_Player_KeyDownAttack::K2_EndAbility);
    Task->ReadyForActivation();
}
