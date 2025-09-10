#include "Player/UI/BaseMonsterHUDWidget.h"
#include "MonsterAttributeSet.h"

#include "Player/UI/BasePlayerStatBarWidget.h"

void UBaseMonsterHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::Collapsed);
}

void UBaseMonsterHUDWidget::InitASC(UAbilitySystemComponent* _OwnerASC, const UAttributeSet* _OwnerAttrSet)
{
	UBasePlayerHUDWidget::InitASC(_OwnerASC, _OwnerAttrSet);

	MonsterAttrSet = Cast<UMonsterAttributeSet>(_OwnerAttrSet);

	if (!OwnerASC.Get() || !MonsterAttrSet.Get()) return;

	CurrentHPChangedDelegateHandle =
		OwnerASC->GetGameplayAttributeValueChangeDelegate(MonsterAttrSet->GetHealthAttribute())
		.AddUObject(this, &UBaseMonsterHUDWidget::OnCurrentHPChanged);

	SetVisibility(ESlateVisibility::Visible);
}

void UBaseMonsterHUDWidget::OnCurrentHPChanged(const FOnAttributeChangeData& _Data)
{
	if (HPBarWidget && MonsterAttrSet.Get())
	{
		HPBarWidget->SetRatio(MonsterAttrSet->GetHealth() / MonsterAttrSet->GetMaxHealth());
	}

	if (MonsterAttrSet->GetHealth() / MonsterAttrSet->GetMaxHealth() <= 0.f ||
		!MonsterAttrSet.IsValid())
	{
		if (IsInViewport())
			RemoveFromParent();
		SetVisibility(ESlateVisibility::Collapsed);
	}
}