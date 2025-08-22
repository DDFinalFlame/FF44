#include "GA_Player_Attack.h"

#include "Components/SphereComponent.h"

#include "Player/BasePlayer.h"
#include "Weapon/BaseWeapon.h"

UGA_Player_Attack::UGA_Player_Attack()
{
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Player.Attack"));
	ActivationRequiredTags.AddTag(FGameplayTag::RequestGameplayTag("Player.Weapon.Equip"));
}

void UGA_Player_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
										const FGameplayAbilityActorInfo* ActorInfo, 
										const FGameplayAbilityActivationInfo ActivationInfo, 
										const FGameplayEventData* TriggerEventData)
{
	if (ABasePlayer* player = Cast<ABasePlayer>(ActorInfo->AvatarActor.Get()))
	{
		OwnerPlayer = player;
		OwnerWeapon = OwnerPlayer->GetWeapon();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
}

void UGA_Player_Attack::CommitExecute(const FGameplayAbilitySpecHandle Handle, 
									  const FGameplayAbilityActorInfo* ActorInfo, 
									  const FGameplayAbilityActivationInfo ActivationInfo)
{

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

	//if (GEngine)
	//{
	//	GEngine->AddOnScreenDebugMessage(
	//		-1,                        // Key (-1 = �� �޽��� ��� �߰�)
	//		5.f,                       // Duration (5��)
	//		FColor::Green,             // ����
	//		FString::Printf(TEXT("EndAbility: %s"), *GetName())
	//	);
	//}
}