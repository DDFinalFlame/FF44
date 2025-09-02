#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

//class ItemBase;

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
	float TileSize;

//	// Items
//	TArray<FItemRow*> Items;
//
//public:
//	bool TryAddItem(ItemBase* _ItemToAdd);
//	bool IsRoomAvailable(ItemBase* _ItemToAdd, int32 _Index);

protected:
	bool IsTileValid(FIntPoint _Tile);

	FIntPoint IndexToTile(int32 _Index);
	int32 TileToIndex(FIntPoint _Tile);

	//bool GetResultAtIndex(int32 _Index);
	//ItemBase* GetItemAtIndex(int32 _Index);

public:
	int32 GetColums() { return Colums; }
	int32 GetRows() { return Rows; }
	float GetTileSize() { return TileSize; }
};
