#pragma once

#include "Engine/DataTable.h"
#include "ItemRow.generated.h"

class UInventoryComponent;
class UCanvasPanel;
class UPanelSlot;

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Use UMETA(DisplayName = "Use"),
	Equip	UMETA(DisplayName = "Equip"),
};

USTRUCT(BlueprintType)
struct FItemRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY();

	// 키 값으로, 소모용
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Dimension;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* Icon;

	// 비 설정 값
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