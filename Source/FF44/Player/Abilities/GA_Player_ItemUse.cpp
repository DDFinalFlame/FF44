#include "GA_Player_ItemUse.h"

#include "Player/BasePlayer.h"
#include "Player/Data/PlayerTags.h"
#include "Player/Camera/BasePlayerCameraManager.h"
#include "InventorySystem/InventoryComponent.h"

UGA_Player_ItemUse::UGA_Player_ItemUse()
{
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(PlayerTags::Ability_Player_UseItem);
	SetAssetTags(AssetTags);

	BlockAbilitiesWithTag.AddTag(PlayerTags::Ability_Player_Attack);

	ActivationOwnedTags.AddTag(PlayerTags::State_Player_ItemUse);

	ActivationBlockedTags.AddTag(PlayerTags::State_Player_Attack);
	ActivationBlockedTags.AddTag(PlayerTags::State_Player_Dead);
	ActivationBlockedTags.AddTag(PlayerTags::State_Player_Dodge);
	ActivationBlockedTags.AddTag(PlayerTags::State_Player_ItemUse);
	ActivationBlockedTags.AddTag(PlayerTags::State_Player_HitReacting);
	ActivationBlockedTags.AddTag(PlayerTags::State_Player_Weapon_ChangeEquip);
}

void UGA_Player_ItemUse::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
										 const FGameplayAbilityActorInfo* ActorInfo,
										 const FGameplayAbilityActivationInfo ActivationInfo,
										 const FGameplayEventData* TriggerEventData)
{
	if (ABasePlayer* player = Cast<ABasePlayer>(ActorInfo->AvatarActor.Get()))
	{
		OwnerPlayer = player;

		// Check Equip State
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			if (ASC->HasMatchingGameplayTag(PlayerTags::State_Player_Weapon_Equip))
			{
				SetUnEquipMode();
				EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
				return;
			}
		}

		// Get InventoryComponent
		if (auto inventory = OwnerPlayer->GetInventoryComponent())
			IC = inventory;
		else
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Find Item
	if (!FindItem())
	{
		DontFindItem();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
}

void UGA_Player_ItemUse::CommitExecute(const FGameplayAbilitySpecHandle Handle,
									   const FGameplayAbilityActorInfo* ActorInfo,
									   const FGameplayAbilityActivationInfo ActivationInfo)
{
}

void UGA_Player_ItemUse::EndAbility(const FGameplayAbilitySpecHandle Handle,
									const FGameplayAbilityActorInfo* ActorInfo,
									const FGameplayAbilityActivationInfo ActivationInfo,
									bool bReplicateEndAbility, bool bWasCancelled)
{

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Player_ItemUse::OnNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
}

void UGA_Player_ItemUse::OnNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
}

void UGA_Player_ItemUse::OnCompleted()
{
	K2_EndAbility();
}

void UGA_Player_ItemUse::OnBlendIn()
{
	K2_EndAbility();
}

void UGA_Player_ItemUse::OnBlendOut()
{
	K2_EndAbility();
}

void UGA_Player_ItemUse::OnInterrupted()
{
	K2_EndAbility();
}

void UGA_Player_ItemUse::OnCancelled()
{
	K2_EndAbility();
}

bool UGA_Player_ItemUse::FindItem()
{
	return true;
}

void UGA_Player_ItemUse::DontFindItem()
{

}

void UGA_Player_ItemUse::SetUnEquipMode()
{
	OwnerPlayer->GetAbilitySystemComponent()->TryActivateAbilityByClass(UnEquipAbililtyClass);

	switch (OwnerPlayer->GetCameraManager()->GetCurrentCameraMode())
	{
	case ECameraMode::UnEquip:
		break;
	case ECameraMode::Equip:
		OwnerPlayer->GetCameraManager()->SetCameraMode(ECameraMode::UnEquip);
		break;
	case ECameraMode::ZoomIn:
		OwnerPlayer->GetCameraManager()->SetCameraMode(ECameraMode::UnEquip);
		break;
	}
}