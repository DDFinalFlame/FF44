#include "Player/BasePlayerAttributeSet.h"
#include "Net/UnrealNetwork.h"

UBasePlayerAttributeSet::UBasePlayerAttributeSet()
{
}

void UBasePlayerAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBasePlayerAttributeSet, Health, OldValue);
}

void UBasePlayerAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBasePlayerAttributeSet, Stamina, OldStamina);
}

void UBasePlayerAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBasePlayerAttributeSet, AttackPower, OldValue);
}

void UBasePlayerAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UBasePlayerAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasePlayerAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasePlayerAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
}