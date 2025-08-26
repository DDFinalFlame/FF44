#include "Player/UI/BasePlayerHUDWidget.h"
#include "AbilitySystemComponent.h"
#include "Player/BasePlayerAttributeSet.h"

void UBasePlayerHUDWidget::InitASC(UAbilitySystemComponent* _OwnerASC, const UBasePlayerAttributeSet* _OwnerAttrSet)
{
	OwnerASC = _OwnerASC;
	OwnerAttrSet = _OwnerAttrSet;
}