#include "InventorySystem/Widget/InventoryGridWidget.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

#include "Player/BasePlayer.h"
#include "InventorySystem/InventoryComponent.h"

void UInventoryGridWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CharRef = Cast<ABasePlayer>(GetOwningPlayerPawn());
	if (!CharRef) 
	{ 
		UE_LOG(LogTemp, Warning, TEXT("Failed to NativeConstruct in %s"), *GetName());
		return; 
	}

	IC = CharRef->GetInventoryComponent();
	if (!IC) 
	{ 
		UE_LOG(LogTemp, Warning, TEXT("Failed to NativeConstruct in %s"), *GetName());
		return; 
	}

	Colums = IC->GetColums();
	Rows = IC->GetRows();
	TileSize = IC->GetTileSize();

	float NewWidth = Colums * TileSize;
	float NewHeight = Rows * TileSize;

	LineStructData.XLines = {};
	LineStructData.YLines = {};

	StartX = {};
	StartY = {};
	EndX = {};
	EndY = {};

	// GridBorder를 슬롯으로 설정해서 크기를 변경한다.
	UCanvasPanelSlot* BorderAsCanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(GridBorder);
	BorderAsCanvasSlot->SetSize(FVector2D(NewWidth, NewHeight));

	CreateLineSegments();
}

void UInventoryGridWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

int32 UInventoryGridWidget::NativePaint(const FPaintArgs& Args,
										const FGeometry& AllottedGeometry,
										const FSlateRect& MyCullingRect,
										FSlateWindowElementList& OutDrawElements,
										int32 LayerId, const FWidgetStyle& InWidgetStyle,
										bool bParentEnabled) const
{
	Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	FPaintContext PaintContext(AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	FLinearColor CustomColor(0.5f, 0.5f, 0.5f, 0.5f);
	FVector2D TopLeftCorner = GridBorder->GetCachedGeometry().GetLocalPositionAtCoordinates(FVector2D(0.f, 0.f));

	for (int32 i = 0; i < LineStructData.XLines.Num(); i++)
	{
		UWidgetBlueprintLibrary::DrawLine(PaintContext, 
			FVector2D(StartX[i], StartY[i]) + TopLeftCorner,
			FVector2D(EndX[i], EndY[i]) + TopLeftCorner,
			CustomColor);
	}

	return int32();
}

void UInventoryGridWidget::CreateLineSegments()
{
	for (int32 iCol = 0; iCol <= Colums; iCol++)
	{
		float x = iCol * TileSize;

		LineStructData.XLines.Add(FVector2D(x, x));
		LineStructData.YLines.Add(FVector2D(0.f, Rows * TileSize));
	}

	for (int32 iRow = 0; iRow <= Rows; iRow++)
	{
		float y = iRow * TileSize;

		LineStructData.YLines.Add(FVector2D(y, y));
		LineStructData.XLines.Add(FVector2D(0.f, Colums * TileSize));
	}

	for (FVector2D Elements : LineStructData.XLines)
	{
		StartX.Add(Elements.X);
		EndX.Add(Elements.Y);
	}

	for (FVector2D Elements : LineStructData.YLines)
	{
		StartY.Add(Elements.X);
		EndY.Add(Elements.Y);
	}
}

void UInventoryGridWidget::RefreshInventory()
{
	TArray<FItemRow*> Keys;
	IC->GetAllItems().GetKeys(Keys);

	if (ItemWidgetClass)
	{
		ItemWidget = CreateWidget(GetWorld, ItemWidgetClass);

		for (FItemRow* item : Keys)
		{

		}
	}
}
