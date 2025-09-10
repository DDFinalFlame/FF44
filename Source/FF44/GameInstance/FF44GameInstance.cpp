#include "GameInstance/FF44GameInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "AbilitySystemComponent.h"
#include "InventorySystem/InventoryComponent.h"

void FPlayerCompState::CaptureFrom(UAbilitySystemComponent* _ASC, UInventoryComponent* _IC)
{
    if (_ASC)
    {

    }

    if (_IC)
    {
        Items = _IC->GetItems();
        AllItems = _IC->GetAllItems();
    }
}

void FPlayerCompState::ApplyTo(UAbilitySystemComponent* _ASC, UInventoryComponent* _IC) const
{
    if (_ASC)
    {

    }

    if (_IC && !Items.IsEmpty() && !AllItems.IsEmpty())
    {
        _IC->SetItems(Items);
        _IC->SetAllItems(AllItems);
    }
}

void UFF44GameInstance::AddGold(int32 Amount)
{
    Gold = FMath::Max(0, Gold + Amount);
}

bool UFF44GameInstance::SpendGold(int32 Amount)
{
    if (Amount <= 0) return true;
    if (Gold < Amount) return false;

    Gold -= Amount;
    return true;
}

void UFF44GameInstance::AddItem(const FItemInstance& Item)
{
    int32 Index = Stash.IndexOfByPredicate([&](const FItemInstance& I)
        {
            return I.ItemId == Item.ItemId && I.Price == Item.Price;
        });

    if (Index != INDEX_NONE)
    {
        Stash[Index].Quantity += Item.Quantity;
    }
    else
    {
        Stash.Add(Item);
    }
}

bool UFF44GameInstance::RemoveItemById(FName ItemId)
{
    int32 Index = Stash.IndexOfByPredicate([&](const FItemInstance& I)
        {
            return I.ItemId == ItemId;
        });

    if (Index != INDEX_NONE)
    {
        Stash.RemoveAt(Index);
        return true;
    }

    return false;
}
