#include "Player/UI/BasePlayerStatBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"

void UBasePlayerStatBarWidget::NativePreConstruct()
{
    Super::NativePreConstruct();

    if (StatBar)
    {
        StatBar->SetFillColorAndOpacity(FillColorAndOpacity);
    }

    if (StatOutline)
    {
        FSlateBrush Brush = StatOutline->GetBrush();

        Brush.DrawAs = ESlateBrushDrawType::Box;     

        StatOutline->SetBrush(Brush);
    }
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