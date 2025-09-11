#include "InventorySystem/Widget/InventoryGridWidget.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"

#include "Player/BasePlayer.h"
#include "InventorySystem/InventoryComponent.h"
#include "InventorySystem/InventorySystemInterface.h"
#include "InventorySystem/Widget/ItemWidget.h"
#include "InventorySystem/Widget/InventoryWidget.h"

int32 UInventoryGridWidget::NativePaint(const FPaintArgs& Args,
										const FGeometry& AllottedGeometry,
										const FSlateRect& MyCullingRect,
										FSlateWindowElementList& OutDrawElements,
										int32 LayerId, const FWidgetStyle& InWidgetStyle,
										bool bParentEnabled) const
{
	Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	FPaintContext PaintContext(AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	FLinearColor CustomColor(0.5f, 0.5f, 0.5f, 0.f);
	FVector2D TopLeftCorner = GridBorder->GetCachedGeometry().GetLocalPositionAtCoordinates(FVector2D(0.f, 0.f));

	for (int32 i = 0; i < LineStructData.XLines.Num(); i++)
	{
		UWidgetBlueprintLibrary::DrawLine(PaintContext,
			FVector2D(StartX[i], StartY[i]) + TopLeftCorner,
			FVector2D(EndX[i], EndY[i]) + TopLeftCorner,
			CustomColor);
	}

	if (DrawDropLocation)
	{
		if (auto wrapper = Cast<UItemDataWrapper>(DraggedPayload))
			if (auto item = wrapper->Data)
				if (IsRoomAvailableForPayload(item))
					DrawBackgroundBox(item, FLinearColor(0.f, 1.f, 0.f, 0.25f), AllottedGeometry, TopLeftCorner, OutDrawElements, LayerId);
				else
					DrawBackgroundBox(item, FLinearColor(1.f, 0.f, 0.f, 0.25f), AllottedGeometry, TopLeftCorner, OutDrawElements, LayerId);
	}

	return int32();
}

void UInventoryGridWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);

	if (!InOperation->Payload) return;

	DraggedPayload = InOperation->Payload;
	DrawDropLocation = true;
}

void UInventoryGridWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);

	DraggedPayload = nullptr;
	DrawDropLocation = false;
}

bool UInventoryGridWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (!InOperation->Payload) return false;

	UItemDataWrapper* wrapper = Cast<UItemDataWrapper>(InOperation->Payload);

	if (auto item = wrapper->Data)
	{
		FVector2D ScreenPosition = InDragDropEvent.GetScreenSpacePosition();
		FVector2D LocalPosition = InGeometry.AbsoluteToLocal(ScreenPosition);

		FVector2D GridStartCoordinates = GridBorder->GetCachedGeometry().GetLocalPositionAtCoordinates(FVector2D(0.f, 0.f));
		FVector2D AdjustedPosition = LocalPosition - GridStartCoordinates;

		FIntPoint ResultTile;
		auto position = MousePositionInTileResult(AdjustedPosition);

		if (position.Right)
			ResultTile.X = FMath::Clamp(item->Dimension.X - 1, 0, item->Dimension.X - 1);
		else
			ResultTile.X = FMath::Clamp(item->Dimension.X, 0, item->Dimension.X);

		if (position.Down)
			ResultTile.Y = FMath::Clamp(item->Dimension.Y - 1, 0, item->Dimension.Y - 1);
		else
			ResultTile.Y = FMath::Clamp(item->Dimension.Y, 0, item->Dimension.Y);

		DraggedItemTile = FIntPoint(FMath::TruncToInt32(AdjustedPosition.X / IC->GetTileSize()),
			FMath::TruncToInt32(AdjustedPosition.Y / IC->GetTileSize()))
			- (ResultTile / 2);

		return true;
	}

	return false;
}

bool UInventoryGridWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (!InOperation->Payload) return false;

	UItemDataWrapper* wrapper = Cast<UItemDataWrapper>(InOperation->Payload);

	if (auto item = wrapper->Data)
	{
		// 놓으려는 곳에 자리가 있으면
		if (IsRoomAvailableForPayload(item))
		{
			IC->AddItemAt(item, IC->TileToIndex(DraggedItemTile));
			DrawItemWidget(item);
		}
		else
		{
			item->OwnerInventory->AddItemAt(item, item->ItemIndex);
			DrawItemWidget(item, item->OwnerPanel, item->OwnerGrid);
		}

		DrawDropLocation = false;
		return true;
	}

	return false;
}

bool UInventoryGridWidget::IsRoomAvailableForPayload(FItemRow* _Item) const
{
	if (!_Item) return false;

	return IC->IsRoomAvailable(_Item, IC->TileToIndex(DraggedItemTile));
}

bool UInventoryGridWidget::DrawInventoryGrid(AActor* _InventoryOwner)
{
	if (!_InventoryOwner)
	{
		UE_LOG(LogTemp, Log, TEXT("Inventory Owner is null"));
		IC = nullptr;
		return false;
	}

	if (_InventoryOwner->GetClass()->HasAnyClassFlags(CLASS_Abstract))
	{
		UE_LOG(LogTemp, Warning, TEXT("DrawInventoryGrid: Owner class is Abstract (%s)"),
			*_InventoryOwner->GetClass()->GetName());
		IC = nullptr;
		return false;
	}

	if (!_InventoryOwner->GetClass()->
		ImplementsInterface(UInventorySystemInterface::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("DrawInventoryGrid: Owner (%s) does NOT implement InventorySystemInterface"),
			*_InventoryOwner->GetName());
		IC = nullptr;
		return false;
	}

	if (IInventorySystemInterface* interface = Cast<IInventorySystemInterface>(_InventoryOwner))
	{
		IC = interface->GetInventoryComponent();

		if (!IC)
		{
			UE_LOG(LogTemp, Warning, TEXT("DrawInventoryGrid: InventoryComponent is null"),
				*_InventoryOwner->GetName());
			return false;
		}
	}

	Colums = IC->GetColums();
	Rows = IC->GetRows();
	TileSize = IC->GetTileSize();

	if (Colums == 0 || Rows == 0) return false;

	LineStructData.XLines = {};
	LineStructData.YLines = {};

	StartX = {};
	StartY = {};
	EndX = {};
	EndY = {};

	// GridBorder를 슬롯으로 설정해서 크기를 변경한다.
	UCanvasPanelSlot* BorderAsCanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(GridBorder);
	BorderAsCanvasSlot->SetSize(FVector2D(Colums, Rows) * TileSize);

	CreateLineSegments();
	CreateGridImage();

	return true;
}

void UInventoryGridWidget::DestroyInventoryGrid()
{
	for (int32 i=0;i< Grids.Num();i++)
	{
		if (Grids[i])
		{
			Grids[i]->RemoveFromParent();
			Grids[i] = nullptr;
		}
	}

	Grids.Reset();
}

void UInventoryGridWidget::DrawItemWidget(FItemRow* _Item)
{
	auto ItemWidget = CreateWidget<UItemWidget>(GetOwningPlayer(), ItemWidgetClass);

	// 인벤토리 꺼지면 삭제
	InventoryWidget->OnNativeVisibilityChanged.AddUObject(ItemWidget, &UItemWidget::OnInvVisibility);

	ItemWidget->SetInventoryComponent(IC);
	ItemWidget->DrawItem(_Item, TileSize);

	int32 x = IC->GetAllItems()[_Item].X * IC->GetTileSize();
	int32 y = IC->GetAllItems()[_Item].Y * IC->GetTileSize();

	PanelSlot = GridCanvasPanel->AddChild(ItemWidget);
	Cast<UCanvasPanelSlot>(PanelSlot)->SetAutoSize(true);
	Cast<UCanvasPanelSlot>(PanelSlot)->SetPosition(FVector2D(x, y));

	_Item->OwnerGrid = GridCanvasPanel;
	_Item->OwnerPanel = PanelSlot;
}

void UInventoryGridWidget::DrawItemWidget(FItemRow* _Item, UPanelSlot* _OtherPanel, UCanvasPanel* _OtherGrid)
{
	auto ItemWidget = CreateWidget<UItemWidget>(GetOwningPlayer(), ItemWidgetClass);

	// 인벤토리 꺼지면 삭제
	InventoryWidget->OnNativeVisibilityChanged.AddUObject(ItemWidget, &UItemWidget::OnInvVisibility);

	ItemWidget->SetInventoryComponent(_Item->OwnerInventory);
	ItemWidget->DrawItem(_Item, TileSize);

	int32 x = _Item->OwnerInventory->GetAllItems()[_Item].X * _Item->OwnerInventory->GetTileSize();
	int32 y = _Item->OwnerInventory->GetAllItems()[_Item].Y * _Item->OwnerInventory->GetTileSize();

	_OtherPanel = _OtherGrid->AddChild(ItemWidget);
	Cast<UCanvasPanelSlot>(_OtherPanel)->SetAutoSize(true);
	Cast<UCanvasPanelSlot>(_OtherPanel)->SetPosition(FVector2D(x, y));
}

void UInventoryGridWidget::DrawItemWidgets()
{
	if (!IC) return;
	if (Colums == 0 || Rows == 0) return;

	TArray<FItemRow*> Keys;
	IC->GetAllItems().GetKeys(Keys);

	if (ItemWidgetClass)
	{
		for (FItemRow* item : Keys)
		{
			DrawItemWidget(item);
		}
	}
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

void UInventoryGridWidget::CreateGridImage()
{
	if (!GridCanvasPanel || !GridImageClass) return;

	UCanvasPanelSlot* BGSlot = Cast<UCanvasPanelSlot>(GridBackground->Slot);
	BGSlot->SetSize(FVector2D((TileSize * Colums) + 40, (TileSize * Rows) + 40));

	for (int32 iCol = 0; iCol < Colums; iCol++)
	{
		for (int32 iRow = 0; iRow < Rows; iRow++)
		{
			auto GridImageWidget = CreateWidget<UUserWidget>(GetOwningPlayer(), GridImageClass);
			GridImageWidget->SetOwningPlayer(GetOwningPlayer());

			UCanvasPanelSlot* CanvasSlot = GridCanvasPanel->AddChildToCanvas(GridImageWidget);

			// 배치/정렬/크기 설정 (원하시는 값으로 조정)
			CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));       // 좌상단 기준
			CanvasSlot->SetAlignment(FVector2D(0.f, 0.f));              // 좌상단 정렬
			CanvasSlot->SetPosition(FVector2D((iCol) * (TileSize), (iRow) * (TileSize)));
			CanvasSlot->SetSize(FVector2D(TileSize, TileSize));

			Grids.Add(GridImageWidget);
		}
	}
}

// Drop 했을 때 어떤 타일에 들어갈지 Position을 계산한다.
FMousePositionInTile UInventoryGridWidget::MousePositionInTileResult(FVector2D _MousePosition)
{
	FMousePositionInTile MousePositionTile;

	MousePositionTile.Right = fmod(_MousePosition.X, IC->GetTileSize()) > (IC->GetTileSize() / 2.f);
	MousePositionTile.Down = fmod(_MousePosition.Y, IC->GetTileSize()) > (IC->GetTileSize() / 2.f);

	return MousePositionTile;
}

void UInventoryGridWidget::DrawBackgroundBox(FItemRow* _Item, FLinearColor _Color, const FGeometry& _AllottedGeometry, FVector2D TopLeftCorner, FSlateWindowElementList& OutDrawElements, int32 LayerId) const
{
	FSlateBrush BoxBrush;
	BoxBrush.DrawAs = ESlateBrushDrawType::Box;

	FVector2D BoxSize(_Item->Dimension.X * TileSize, _Item->Dimension.Y * TileSize);
	FIntPoint BoxPoisition(DraggedItemTile.X * TileSize, DraggedItemTile.Y * TileSize);

	FPaintGeometry PaintGeometry = _AllottedGeometry.ToPaintGeometry(BoxSize, FSlateLayoutTransform(TopLeftCorner + BoxPoisition));

	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, PaintGeometry, &BoxBrush, ESlateDrawEffect::None, _Color);
}
