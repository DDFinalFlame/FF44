#include "GA_Player_Attack.h"

#include "Components/SphereComponent.h"

#include "Player/BasePlayer.h"
#include "Weapon/BaseWeapon.h"

UGA_Player_Attack::UGA_Player_Attack()
{
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("Player.Attack"));
	ActivationRequiredTags.AddTag(FGameplayTag::RequestGameplayTag("Player.Weapon.Equip"));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Player.Dodge"));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("Player.Hit"));
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

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,                        // Key (-1 = 새 메시지 계속 추가)
			5.f,                       // Duration (5초)
			FColor::Green,             // 색상
			FString::Printf(TEXT("EndAbility: %s"), *GetName())
		);
	}
}

void UGA_Player_Attack::OnAttack_Implementation()
{

}
