#include "Player/UI/BasePlayerHUDWidget.h"
#include "AbilitySystemComponent.h"

void UBasePlayerHUDWidget::InitASC(UAbilitySystemComponent* _OwnerASC, const UAttributeSet* _OwnerAttrSet)
{
	OwnerASC = _OwnerASC;
	OwnerAttrSet = _OwnerAttrSet;
}