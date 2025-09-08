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

void UPlayerInGameHUDWidget::InitASC(UAbilitySystemComponent* _OwnerASC, const UAttributeSet* _OwnerAttrSet)
{
	UBasePlayerHUDWidget::InitASC(_OwnerASC, _OwnerAttrSet);

	PlayerAttrSet = Cast<UBasePlayerAttributeSet>(_OwnerAttrSet);

	if (!OwnerASC.Get() || !PlayerAttrSet.Get()) return;

	CurrentHPChangedDelegateHandle =
		OwnerASC->GetGameplayAttributeValueChangeDelegate(PlayerAttrSet->GetCurrentHPAttribute())
		.AddUObject(this, &UPlayerInGameHUDWidget::OnCurrentHPChanged);

	CurrentStaminaChangedDelegateHandle =
		OwnerASC->GetGameplayAttributeValueChangeDelegate(PlayerAttrSet->GetCurrentStaminaAttribute())
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
	if (HPBarWidget && PlayerAttrSet.Get())
	{
		HPBarWidget->SetRatio(PlayerAttrSet->GetCurrentHP() / PlayerAttrSet->GetMaxHP());
	}
}

void UPlayerInGameHUDWidget::OnCurrentStaminaChanged(const FOnAttributeChangeData& _Data)
{
	if (StaminaBarWidget && PlayerAttrSet.Get())
	{
		StaminaBarWidget->SetRatio(PlayerAttrSet->GetCurrentStamina() / PlayerAttrSet->GetMaxStamina());
	}
}
