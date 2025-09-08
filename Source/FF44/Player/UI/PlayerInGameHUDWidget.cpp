#include "Player/UI/PlayerInGameHUDWidget.h"
#include "AbilitySystemComponent.h"
#include "Player/BasePlayerAttributeSet.h"

#include "Player/UI/BasePlayerStatBarWidget.h"

void UPlayerInGameHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ProgressBarWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void UPlayerInGameHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	if (IsProgress)
	{
		if (ProgressBarWidget)
		{
			ProgressBarWidget->SetRatio(CurrentTime / MaxTime);
		}

		if (CurrentTime < MaxTime)
			CurrentTime += InDeltaTime;
		else
			EndProgressBar();
	}
}

void UPlayerInGameHUDWidget::InitASC(UAbilitySystemComponent* _OwnerASC, const UBasePlayerAttributeSet* _OwnerAttrSet)
{
	UBasePlayerHUDWidget::InitASC(_OwnerASC, _OwnerAttrSet);

	if (!OwnerASC || !OwnerAttrSet) return;

	CurrentHPChangedDelegateHandle =
		OwnerASC->GetGameplayAttributeValueChangeDelegate(OwnerAttrSet->GetCurrentHPAttribute())
		.AddUObject(this, &UPlayerInGameHUDWidget::OnCurrentHPChanged);

	CurrentStaminaChangedDelegateHandle =
		OwnerASC->GetGameplayAttributeValueChangeDelegate(OwnerAttrSet->GetCurrentStaminaAttribute())
		.AddUObject(this, &UPlayerInGameHUDWidget::OnCurrentStaminaChanged);
}

void UPlayerInGameHUDWidget::SetProgressBar(float _MaxTime)
{
	ProgressBarWidget->SetVisibility(ESlateVisibility::Visible);

	MaxTime = _MaxTime;
	CurrentTime = 0.f;
	IsProgress = true;
}

void UPlayerInGameHUDWidget::EndProgressBar()
{
	ProgressBarWidget->SetVisibility(ESlateVisibility::Collapsed);

	MaxTime = 0.f;
	CurrentTime = 0.f;
	IsProgress = false;
}

void UPlayerInGameHUDWidget::OnCurrentHPChanged(const FOnAttributeChangeData& _Data)
{
	if (HPBarWidget && OwnerAttrSet)
	{
		HPBarWidget->SetRatio(OwnerAttrSet->GetCurrentHP() / OwnerAttrSet->GetMaxHP());
	}
}

void UPlayerInGameHUDWidget::OnCurrentStaminaChanged(const FOnAttributeChangeData& _Data)
{
	if (StaminaBarWidget && OwnerAttrSet)
	{
		StaminaBarWidget->SetRatio(OwnerAttrSet->GetCurrentStamina() / OwnerAttrSet->GetMaxStamina());
	}
}
