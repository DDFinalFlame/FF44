#include "Player/UI/BasePlayerStatBarWidget.h"
#include "Components/ProgressBar.h"

void UBasePlayerStatBarWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (!StatBar) return;

	StatBar->SetFillColorAndOpacity(FillColorAndOpacity);
}

void UBasePlayerStatBarWidget::SetRatio(float Ratio)
{
	if (StatBar)
	{
		StatBar->SetPercent(Ratio);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("StatBar is not bound!"));
	}
}