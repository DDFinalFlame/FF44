#include "GA_Player_Attack.h"

#include "Components/SphereComponent.h"

#include "Player/BasePlayer.h"
#include "Weapon/BaseWeapon.h"

UGA_Player_Attack::UGA_Player_Attack()
{
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("Player.Attack"));
	ActivationRequiredTags.AddTag(FGameplayTag::RequestGameplayTag("Player.Equip"));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Player.Dodge"));
}

void UGA_Player_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
										const FGameplayAbilityActorInfo* ActorInfo, 
										const FGameplayAbilityActivationInfo ActivationInfo, 
										const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!OwnerPlayer)
	{
		OwnerPlayer = Cast<ABasePlayer>(ActorInfo->AvatarActor.Get());
	}

	if (!OwnerWeapon)
	{
		if (OwnerPlayer)
		{
			OwnerWeapon = OwnerPlayer->GetWeapon();
		}
	}

	OnAttack();
}

void UGA_Player_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle, 
								   const FGameplayAbilityActorInfo* ActorInfo, 
								   const FGameplayAbilityActivationInfo ActivationInfo, 
								   bool bReplicateEndAbility, bool bWasCancelled)
{
	if (OwnerWeapon)
	{
		OwnerWeapon->GetWeaponCollision()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Player_Attack::OnAttack_Implementation()
{

}
