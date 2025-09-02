#include "InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Begin InventoryComponent"));

	//Items.SetNum(Colums, Rows);
}

void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

//bool UInventoryComponent::TryAddItem(class ItemBase* _ItemToAdd)
//{
//	if (_ItemToAdd)	{
//		for (int32 i = 0; i < Items.Num(); i++)	{
//
//		}
//	}
//
//	return false;
//}
//
//bool UInventoryComponent::IsRoomAvailable(ItemBase* _ItemToAdd, int32 _Index)
//{
//	return false;
//}

bool UInventoryComponent::IsTileValid(FIntPoint _Tile)
{
	if (_Tile.X >= 0 && _Tile.Y >= 0 && _Tile.X < Colums && _Tile.Y < Rows)
		return true;

	return false;
}

// Item �������(���)�� Index -> FIntPoint�� ��ȯ
FIntPoint UInventoryComponent::IndexToTile(int32 _Index)
{
	return FIntPoint(_Index % Colums, _Index / Colums);
}

// Item �������(���)�� FIntPoint -> Index�� ��ȯ
int32 UInventoryComponent::TileToIndex(FIntPoint _Tile)
{
	return int32(_Tile.X + (_Tile.Y * Colums));
}

//// Index�� Item�� �ִ����� Ȯ���Ѵ�.
//bool UInventoryComponent::GetResultAtIndex(int32 _Index)
//{
//	if (Items.IsValidIndex(_Index))
//		return true;
//
//	return false;
//}
//
//// Index�� Item�� �����´�
//ItemBase* UInventoryComponent::GetItemAtIndex(int32 _Index)
//{
//	if (Items.IsValidIndex(_Index))
//		return Items[_Index];
//
//	return nullptr;
//}