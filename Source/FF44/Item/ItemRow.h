#pragma once

#include "Engine/DataTable.h"
#include "ItemRow.generated.h"

class UInventoryComponent;
class UCanvasPanel;
class UPanelSlot;

USTRUCT(BlueprintType)
struct FItemRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Dimension;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* Icon;

	// �� ���� ��
	int32 ItemIndex;
	UInventoryComponent* OwnerInventory;
	UCanvasPanel* OwnerGrid;
	UPanelSlot* OwnerPanel;
};

UCLASS(BlueprintType)
class UItemDataWrapper : public UObject
{
	GENERATED_BODY()

public:
	FItemRow* Data;
};