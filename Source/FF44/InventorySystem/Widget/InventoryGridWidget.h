#pragma once

#include "InventorySystem/InventoryDataStructs.h"

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryGridWidget.generated.h"

class UCanvasPanel;
class UBorder;

struct FLines;

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

	int32 Colums;
	int32 Rows;
	float TileSize;

	UPROPERTY()
	FLines LineStructData;

	TArray<float> StartX;
	TArray<float> StartY;
	TArray<float> EndX;
	TArray<float> EndY;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual int32 NativePaint(const FPaintArgs& Args, 
							  const FGeometry& AllottedGeometry,
							  const FSlateRect& MyCullingRect, 
							  FSlateWindowElementList& OutDrawElements,
							  int32 LayerId, const FWidgetStyle& InWidgetStyle,
							  bool bParentEnabled) const override;

	void CreateLineSegments();
};
