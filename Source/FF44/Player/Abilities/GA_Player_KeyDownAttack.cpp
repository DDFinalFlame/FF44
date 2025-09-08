#include "GA_Player_KeyDownAttack.h"

#include "Components/SphereComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameplayTagContainer.h"

#include "Player/BasePlayer.h"
#include "Weapon/BaseWeapon.h"
#include "Player/Data/PlayerTags.h"

UGA_Player_KeyDownAttack::UGA_Player_KeyDownAttack()
{
	//UGA_Player_Attack::UGA_Player_Attack();

    ActivationOwnedTags.AddTag(PlayerTags::State_Player_KeyDownAttack);
}

void UGA_Player_KeyDownAttack::CommitExecute(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    UAbilityTask_PlayMontageAndWait* Task =
        UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this,      
            NAME_None, 
            StartAttackMontage, 
            MontagePlayRate, 
            NAME_None,    
            false,        
            1.0f          
        );

    Task->OnCompleted.AddDynamic(this, &UGA_Player_KeyDownAttack::LoopAttack);
    Task->OnBlendOut.AddDynamic(this, &UGA_Player_KeyDownAttack::LoopAttack);
    Task->OnInterrupted.AddDynamic(this, &UGA_Player_KeyDownAttack::K2_EndAbility);
    Task->OnCancelled.AddDynamic(this, &UGA_Player_KeyDownAttack::K2_EndAbility);
    Task->ReadyForActivation();
}

void UGA_Player_KeyDownAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Player_KeyDownAttack::LoopAttack()
{
    if (!OwnerPlayer->IsKeyDownAttack())
    {
        EndAttack();
        return;
    }

    OwnerWeapon->GetWeaponCollision()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

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

    Task->OnCompleted.AddDynamic(this, &UGA_Player_KeyDownAttack::LoopAttack);
    Task->OnBlendOut.AddDynamic(this, &UGA_Player_KeyDownAttack::LoopAttack);
    Task->OnInterrupted.AddDynamic(this, &UGA_Player_KeyDownAttack::K2_EndAbility);
    Task->OnCancelled.AddDynamic(this, &UGA_Player_KeyDownAttack::K2_EndAbility);
    Task->ReadyForActivation();
}

void UGA_Player_KeyDownAttack::EndAttack()
{
    OwnerWeapon->GetWeaponCollision()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    UAbilityTask_PlayMontageAndWait* Task =
        UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this,
            NAME_None,
            EndAttackMontage,
            MontagePlayRate,
            NAME_None,
            false,
            1.0f
        );

    Task->OnCompleted.AddDynamic(this, &UGA_Player_KeyDownAttack::K2_EndAbility);
    Task->OnBlendOut.AddDynamic(this, &UGA_Player_KeyDownAttack::K2_EndAbility);
    Task->OnInterrupted.AddDynamic(this, &UGA_Player_KeyDownAttack::K2_EndAbility);
    Task->OnCancelled.AddDynamic(this, &UGA_Player_KeyDownAttack::K2_EndAbility);
    Task->ReadyForActivation();
}
