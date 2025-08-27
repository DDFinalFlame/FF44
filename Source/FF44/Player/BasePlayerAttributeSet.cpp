#include "Player/BasePlayerAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Data/PlayerTags.h"

UBasePlayerAttributeSet::UBasePlayerAttributeSet()
{
}

void UBasePlayerAttributeSet::OnRep_CurrentHP(const FGameplayAttributeData& _OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBasePlayerAttributeSet, CurrentHP, _OldValue);
}

void UBasePlayerAttributeSet::OnRep_MaxHP(const FGameplayAttributeData& _OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBasePlayerAttributeSet, MaxHP, _OldValue);
}

void UBasePlayerAttributeSet::OnRep_CurrentStamina(const FGameplayAttributeData& _OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBasePlayerAttributeSet, CurrentStamina, _OldValue);
}

void UBasePlayerAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& _OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBasePlayerAttributeSet, MaxStamina, _OldValue);
}

void UBasePlayerAttributeSet::OnRep_RegenRateStamina(const FGameplayAttributeData& _OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBasePlayerAttributeSet, RegenRateStamina, _OldValue);
}

void UBasePlayerAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& _OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBasePlayerAttributeSet, AttackPower, _OldValue);
}

void UBasePlayerAttributeSet::OnRep_DefencePoint(const FGameplayAttributeData& _OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBasePlayerAttributeSet, DefencePoint, _OldValue);
}

void UBasePlayerAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UBasePlayerAttributeSet, CurrentHP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasePlayerAttributeSet, MaxHP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasePlayerAttributeSet, CurrentStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasePlayerAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasePlayerAttributeSet, RegenRateStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasePlayerAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasePlayerAttributeSet, DefencePoint, COND_None, REPNOTIFY_Always);
}

void UBasePlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& _Data)
{
	if (_Data.EvaluatedData.Attribute == GetCurrentHPAttribute())
	{
		// ����!! GE���� CurrentHP�� �ʱ�ȭ �� ��, MaxHP���� ���� ���� �� ���� �������� 0 ��!!
		float newHP = GetCurrentHP();
		newHP = FMath::Clamp(newHP, 0.f, GetMaxHP());
		SetCurrentHP(newHP);

		if(newHP <= 0.f)
		{
			// �÷��̾ �׾��� �� ó��
			AActor* Owner = GetOwningActor();
			if (Owner)
			{
				FGameplayEventData EventData;
				EventData.EventTag = PlayerTags::Event_Player_Death;
				EventData.Instigator = Owner;

				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, PlayerTags::Event_Player_Death, EventData);
			}
		}
	}

	if(_Data.EvaluatedData.Attribute == GetCurrentStaminaAttribute())
	{
		float newStamina = GetCurrentStamina();
		newStamina = FMath::Clamp(newStamina, 0.f, GetMaxStamina());
		SetCurrentStamina(newStamina);
	}
}
