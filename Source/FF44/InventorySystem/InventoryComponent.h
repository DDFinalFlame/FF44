#pragma once

#include "Item/ItemRow.h"

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class UInventoryGridWidget;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FF44_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventoryComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	UPROPERTY(EditAnywhere, Category = "IC Info | Inventory Colums")
	int32 Colums;

	UPROPERTY(EditAnywhere, Category = "IC Info | Inventory Rows")
	int32 Rows;

	UPROPERTY(EditAnywhere, Category = "IC Info | Inventory Tile Size")
	float TileSize = 60.f;

	UInventoryGridWidget* GridWidget;

	// Items
	TArray<FItemRow*> Items;
	TMap<FItemRow*, FIntPoint> AllItems;

public:
	bool TryAddItem(FItemRow* _ItemToAdd);
	bool IsRoomAvailable(FItemRow* _ItemToAdd, int32 _Index);

	void SetItems(TArray<FItemRow*> _Items) { Items = _Items; }
	TArray<FItemRow*> GetItems() { return Items; }

	void SetAllItems(TMap<FItemRow*, FIntPoint> _AllItems);
	TMap<FItemRow*, FIntPoint> GetAllItems() { return AllItems; }

	void AddItemAt(FItemRow* _ItemToAdd, int32 _Index);
	void RemoveItem(FItemRow* _ItemToRemove);

	bool IsTileValid(FIntPoint _Tile);

	FIntPoint IndexToTile(int32 _Index);
	int32 TileToIndex(FIntPoint _Tile);

	bool GetResultAtIndex(int32 _Index);
	FItemRow* GetItemAtIndex(int32 _Index);


public:
	int32 GetColums() { return Colums; }
	int32 GetRows() { return Rows; }
	float GetTileSize() { return TileSize; }
};
