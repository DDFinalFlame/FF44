#include "Player/UI/BasePlayerWidget.h"
#include "AbilitySystemComponent.h"
#include "Player/BasePlayerAttributeSet.h"

void UBasePlayerWidget::InitASC(UAbilitySystemComponent* _OwnerASC, const UBasePlayerAttributeSet* _OwnerAttrSet)
{
	OwnerASC = _OwnerASC;
	OwnerAttrSet = _OwnerAttrSet;
}