#include "InventorySystem/Widget/InventoryWidget.h"
#include "InventoryGridWidget.h"

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PlayerGrid->DrawInventoryGrid(GetOwningPlayerPawn());
		
	OnNativeVisibilityChanged.AddUObject(this, &UInventoryWidget::VisibilityChanged);

	PlayerGrid->SetInventoryWidget(this);
	OtherGrid->SetInventoryWidget(this);
}

FReply UInventoryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Handled();
}

void UInventoryWidget::VisibilityChanged(ESlateVisibility NewVis)
{
	if (NewVis == ESlateVisibility::Visible)
	{
		PlayerGrid->DrawItemWidgets();

		if (OtherActor)
		{
			OtherGrid->DrawInventoryGrid(OtherActor);
			OtherGrid->DrawItemWidgets();
			OtherGrid->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			OtherGrid->DrawInventoryGrid(nullptr);
			OtherGrid->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	else
	{
		OtherGrid->DrawInventoryGrid(nullptr);
	}
}
