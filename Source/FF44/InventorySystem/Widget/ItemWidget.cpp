#include "InventorySystem/Widget/ItemWidget.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/DragDropOperation.h"

#include "Item/ItemRow.h"
#include "InventorySystem/InventoryComponent.h"

void UItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

// Item위에 Mouse가 올라갔을 때 호출
void UItemWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	BackgroundBorder->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.2f));
}

// Item위에 Mouse가 떠날 때 호출
void UItemWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	BackgroundBorder->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.5f));
}

// Item을 드래그 했을 때 호출
void UItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	BackgroundBorder->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.5f));

	UDragDropOperation* DragOperation = NewObject<UDragDropOperation>();
	DragOperation->DefaultDragVisual = this;

	UItemDataWrapper* Wrapper = NewObject<UItemDataWrapper>();
	Wrapper->Data = Item; // struct 복사

	DragOperation->Payload = Wrapper;

	if (IC)
	{
		IC->RemoveItem(Item);
	}

	OutOperation = DragOperation;

	this->RemoveFromParent();
}

// Item을 왼클릭으로 누를 때 호출
FReply UItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
}

void UItemWidget::DrawItem(FItemRow* ItemToAdd, float TileSize)
{
	Item = ItemToAdd;
	ItemImage->SetBrushFromMaterial(ItemToAdd->Icon);

	// Size를 계산해 BackgroundSizeBox와 ItemImage를 변경해준다.
	Size = FVector2D(ItemToAdd->Dimension.X * TileSize,
					 ItemToAdd->Dimension.Y * TileSize);

	BackgroundSizeBox->SetWidthOverride(Size.X);
	BackgroundSizeBox->SetHeightOverride(Size.Y);

	UCanvasPanelSlot* ImageAsCanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(ItemImage);

	ImageAsCanvasSlot->SetSize(Size);
}

void UItemWidget::OnInvVisibility(ESlateVisibility NewVis)
{
	if (NewVis != ESlateVisibility::Visible)
		this->RemoveFromParent();
}