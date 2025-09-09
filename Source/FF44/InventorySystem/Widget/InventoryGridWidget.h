#pragma once

#include "InventorySystem/InventoryDataStructs.h"
#include "Item/ItemRow.h"

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryGridWidget.generated.h"

class UCanvasPanel;
class UBorder;

class UItemWidget;
class UInventoryWidget;

UCLASS()
class FF44_API UInventoryGridWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	UCanvasPanel* Canvas;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	UBorder* GridBorder;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	UCanvasPanel* GridCanvasPanel;

	UPROPERTY(EditAnywhere, Category = "UI | Image")
	TSubclassOf<UUserWidget> GridImageClass;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	UBorder* GridBackground;

	UPROPERTY()
	UPanelSlot* PanelSlot;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UItemWidget> ItemWidgetClass;

	UPROPERTY()
	UInventoryWidget* InventoryWidget;


protected:
	UPROPERTY()
	class UInventoryComponent* IC;

	FLines LineStructData;

	int32 Colums;
	int32 Rows;
	float TileSize;

	TArray<float> StartX;
	TArray<float> StartY;
	TArray<float> EndX;
	TArray<float> EndY;

	FIntPoint DraggedItemTile;

	bool DrawDropLocation = false;
	UObject* DraggedPayload;

protected:
	virtual int32 NativePaint(const FPaintArgs& Args, 
							  const FGeometry& AllottedGeometry,
							  const FSlateRect& MyCullingRect, 
							  FSlateWindowElementList& OutDrawElements,
							  int32 LayerId, const FWidgetStyle& InWidgetStyle,
							  bool bParentEnabled) const override;

	void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	bool IsRoomAvailableForPayload(FItemRow* _Item) const;

public:
	void SetInventoryWidget(UInventoryWidget* _Widget) { InventoryWidget = _Widget; }
	void DrawInventoryGrid(AActor* _InventoryOwner);
	void DrawItemWidget(FItemRow* _Item);
	void DrawItemWidget(FItemRow* _Item, UPanelSlot* _OtherPanel, UCanvasPanel* _OtherGrid);
	void DrawItemWidgets();

protected:
	void CreateLineSegments();
	void CreateGridImage();
	FMousePositionInTile MousePositionInTileResult(FVector2D _MousePosition);
	void DrawBackgroundBox(FItemRow* _Item, FLinearColor _Color, const FGeometry& _AllottedGeometry, FVector2D TopLeftCorner, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;
};
