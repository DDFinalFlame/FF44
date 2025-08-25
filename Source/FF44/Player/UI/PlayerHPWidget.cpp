#include "Player/UI/PlayerHPWidget.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystemComponent.h"
#include "Player/BasePlayerAttributeSet.h"

void UPlayerHPWidget::InitASC(UAbilitySystemComponent* _OwnerASC, const UBasePlayerAttributeSet* _OwnerAttrSet)
{
	UBasePlayerWidget::InitASC(_OwnerASC, _OwnerAttrSet);

	if (!OwnerASC || !OwnerAttrSet) return;


	CurrentHPChangedDelegateHandle = 
		OwnerASC->GetGameplayAttributeValueChangeDelegate(OwnerAttrSet->GetCurrentHPAttribute())
		.AddUObject(this, &UPlayerHPWidget::OnCurrentHPChanged);
	MaxHPChangedDelegateHandle = 
		OwnerASC->GetGameplayAttributeValueChangeDelegate(OwnerAttrSet->GetMaxHPAttribute())
		.AddUObject(this, &UPlayerHPWidget::OnMaxHPChanged);

	UpdateHPUI();
}

void UPlayerHPWidget::OnCurrentHPChanged(const FOnAttributeChangeData& _Data)
{
	UpdateHPUI();
}

void UPlayerHPWidget::OnMaxHPChanged(const FOnAttributeChangeData& _Data)
{
	UpdateHPUI();
}

void UPlayerHPWidget::UpdateHPUI()
{
	if (!OwnerASC || !OwnerAttrSet) return;

	CurrentHP = OwnerASC->GetNumericAttribute(OwnerAttrSet->GetCurrentHPAttribute());
	MaxHP = OwnerASC->GetNumericAttribute(OwnerAttrSet->GetMaxHPAttribute());
}
