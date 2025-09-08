#include "Player/UI/BasePlayerStatBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"

void UBasePlayerStatBarWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	StatBar->SetFillColorAndOpacity(FillColorAndOpacity);

	FSlateBrush Brush = StatOutline->Brush;
	Brush.DrawAs = ESlateBrushDrawType::Box;
	StatOutline->SetBrush(Brush);
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