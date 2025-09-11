#include "InventoryComponent.h"
#include "Widget/InventoryGridWidget.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	Items.SetNum(Colums * Rows);
}

void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// 비어있는 공간이 있는지 확인하고 AddItemAt을 호출하여 item을 추가
bool UInventoryComponent::TryAddItem(FItemRow* _ItemToAdd)
{
	if (_ItemToAdd)	{
		for (int32 i = 0; i < Items.Num(); i++)	{
			if (IsRoomAvailable(_ItemToAdd, i))
			{
				AddItemAt(_ItemToAdd, i);
				return true;
			}
		}
	}

	return false;
}

// Index에 Item Dimension이 맞춰지는지
bool UInventoryComponent::IsRoomAvailable(FItemRow* _ItemToAdd, int32 _Index)
{
	if (Rows == 0 || Colums == 0) return false;

	FIntPoint Dimensions = _ItemToAdd->Dimension;
	FIntPoint Tile = IndexToTile(_Index);

	for (int32 x = Tile.X; x <= Tile.X + Dimensions.X - 1; x++)	{
		for (int32 y = Tile.Y; y <= Tile.Y + Dimensions.Y - 1; y++)	{

			// 인벤토리 내에 존재하는 타일이면,
			if (IsTileValid(FIntPoint(x, y)))
			{
				int32 Index = TileToIndex(FIntPoint(x, y));
				
				// Index가 존재하면, (끝방같이 잘못된 Index 방지)
				if (GetResultAtIndex(Index))
				{
					// Index에 아이템이 있으면,
					if (GetItemAtIndex(Index))
						return false;

				}
				else
					return false;

			}
			else
				return false;
		}
	}

	return true;
}

void UInventoryComponent::SetAllItems(TMap<FItemRow*, FIntPoint> _AllItems)
{
	{ AllItems = _AllItems; }

	for (auto item : AllItems)
			item.Key->OwnerInventory = this;
}

// Index에 Item을 바로 추가
void UInventoryComponent::AddItemAt(FItemRow* _ItemToAdd, int32 _Index)
{
	FIntPoint Dimensions = _ItemToAdd->Dimension;
	FIntPoint Tile = IndexToTile(_Index);
	bool isAdd = false;

	for (int32 x = Tile.X; x <= Tile.X + Dimensions.X - 1; x++) {
		for (int32 y = Tile.Y; y <= Tile.Y + Dimensions.Y - 1; y++) {
			Items[TileToIndex(FIntPoint(x, y))] = _ItemToAdd;

			if (!isAdd)
			{
				AllItems.Add(_ItemToAdd, FIntPoint(x, y));
				_ItemToAdd->ItemIndex = _Index;
				_ItemToAdd->OwnerInventory = this;
				isAdd = true;
			}
		}
	}
}

// Index의 Item을 바로 삭제
void UInventoryComponent::RemoveItem(FItemRow* _ItemToRemove)
{
	if (!_ItemToRemove) return;

	AllItems.Remove(_ItemToRemove);
	for (int32 i = 0; i < Items.Num(); i++)
	{
		if (Items[i] == _ItemToRemove)
		{
			Items[i] = nullptr;
		}
	}
}

// 인벤토리 사이즈를 넘어가지 않는지
bool UInventoryComponent::IsTileValid(FIntPoint _Tile)
{
	if (_Tile.X >= 0 && _Tile.Y >= 0 && _Tile.X < Colums && _Tile.Y < Rows)
		return true;

	return false;
}

// Item 저장공간(헤드)를 Index -> FIntPoint로 변환
FIntPoint UInventoryComponent::IndexToTile(int32 _Index)
{
	return FIntPoint(_Index % Colums, _Index / Colums);
}

// Item 저장공간(헤드)를 FIntPoint -> Index로 변환
int32 UInventoryComponent::TileToIndex(FIntPoint _Tile)
{
	return int32(_Tile.X + (_Tile.Y * Colums));
}

// Index에 Item이 있는지를 확인한다.
bool UInventoryComponent::GetResultAtIndex(int32 _Index)
{
	if (Items.IsValidIndex(_Index))
		return true;

	return false;
}

// Index의 Item을 가져온다
FItemRow* UInventoryComponent::GetItemAtIndex(int32 _Index)
{
	if (Items.IsValidIndex(_Index))
		return Items[_Index];

	return nullptr;
}

bool UInventoryComponent::ConsumeItem(FName _ItemName)
{
	for (auto item : AllItems)
	{
		if (item.Key->ItemName == _ItemName)
		{
			RemoveItem(item.Key);
			return true;
		}
	}

	return false;
}
