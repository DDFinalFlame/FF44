
#include "MonsterAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectExtension.h"  
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "MonsterTags.h"
UMonsterAttributeSet::UMonsterAttributeSet()
{
}

void UMonsterAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMonsterAttributeSet, Health, OldValue);
}

void UMonsterAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMonsterAttributeSet, AttackPower, OldValue);
}

void UMonsterAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMonsterAttributeSet, MoveSpeed, OldValue);
}

void UMonsterAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& _old)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMonsterAttributeSet, MaxHealth, _old);
}

void UMonsterAttributeSet::OnRep_Defense(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMonsterAttributeSet, Defense, OldValue);
}

void UMonsterAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMonsterAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMonsterAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMonsterAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMonsterAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMonsterAttributeSet, Defense, COND_None, REPNOTIFY_Always);
}

/** ���� �ٲ�� ����: Max/Min ���� ���� */
void UMonsterAttributeSet::PreAttributeChange(const FGameplayAttribute& _attr, float& _newValue)
{
	Super::PreAttributeChange(_attr, _newValue);

	if (_attr == GetMaxHealthAttribute())
	{
		// MaxHealth�� �������� Health�� ������ ������
		if (_newValue < 1.f) _newValue = 1.f;
		float clampedHealth = FMath::Clamp(GetHealth(), 0.f, _newValue);
		SetHealth(clampedHealth);
	}
	else if (_attr == GetHealthAttribute())
	{
		float maxH = GetMaxHealth();
		if (maxH < 1.f) maxH = 1.f;
		_newValue = FMath::Clamp(_newValue, 0.f, maxH);
	}
	else if (_attr == GetMoveSpeedAttribute())
	{
		if (_newValue < 0.f) _newValue = 0.f;
	}
	else if (_attr == GetDefenseAttribute())
	{
		// ���� 0���Ϸ� �������°� ���� 
		// ���� - ������ ������, ����
		_newValue = FMath::Max(0.f, _newValue);
	}
}

/** GE ���� ����: Health 0 üũ �� Event.Death 1ȸ �۽� */
void UMonsterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& _data)
{
	Super::PostGameplayEffectExecute(_data);

	if (_data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		float maxH = GetMaxHealth();
		if (maxH < 1.f) maxH = 1.f;

		float newH = GetHealth();
		if (newH < 0.f) newH = 0.f;
		if (newH > maxH) newH = maxH;
		SetHealth(newH);

		// 0�� �Ǿ��� �� Death �̺�Ʈ Ʈ����
		if (newH <= 0.f && _data.Target.AbilityActorInfo.IsValid())
		{
			AActor* avatar = _data.Target.AbilityActorInfo->AvatarActor.Get();
			UAbilitySystemComponent* asc = _data.Target.AbilityActorInfo->AbilitySystemComponent.Get();

			// �̹� Dead ���¸�(���� Ÿ�� �ߺ� ����) ��۽� ����
			if (asc && !asc->HasMatchingGameplayTag(MonsterTags::State_Dead))
			{
				FGameplayEventData evt;
				evt.EventTag = MonsterTags::Event_Death;
				evt.Target = avatar;

				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(avatar, MonsterTags::Event_Death, evt);
			}
		}
	}
}