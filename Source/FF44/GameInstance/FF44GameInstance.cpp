// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/FF44GameInstance.h"
#include "Kismet/KismetMathLibrary.h"

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


