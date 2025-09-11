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

// ����ִ� ������ �ִ��� Ȯ���ϰ� AddItemAt�� ȣ���Ͽ� item�� �߰�
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

// Index�� Item Dimension�� ����������
bool UInventoryComponent::IsRoomAvailable(FItemRow* _ItemToAdd, int32 _Index)
{
	if (Rows == 0 || Colums == 0) return false;

	FIntPoint Dimensions = _ItemToAdd->Dimension;
	FIntPoint Tile = IndexToTile(_Index);

	for (int32 x = Tile.X; x <= Tile.X + Dimensions.X - 1; x++)	{
		for (int32 y = Tile.Y; y <= Tile.Y + Dimensions.Y - 1; y++)	{

			// �κ��丮 ���� �����ϴ� Ÿ���̸�,
			if (IsTileValid(FIntPoint(x, y)))
			{
				int32 Index = TileToIndex(FIntPoint(x, y));
				
				// Index�� �����ϸ�, (���氰�� �߸��� Index ����)
				if (GetResultAtIndex(Index))
				{
					// Index�� �������� ������,
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

// Index�� Item�� �ٷ� �߰�
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

// Index�� Item�� �ٷ� ����
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

// �κ��丮 ����� �Ѿ�� �ʴ���
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

// Index�� Item�� �ִ����� Ȯ���Ѵ�.
bool UInventoryComponent::GetResultAtIndex(int32 _Index)
{
	if (Items.IsValidIndex(_Index))
		return true;

	return false;
}

// Index�� Item�� �����´�
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
