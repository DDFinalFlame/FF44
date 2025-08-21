// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/FF44GameInstance.h"

void UFF44GameInstance::AddGold(int32 Amount)
{
    Gold += Amount;
}

bool UFF44GameInstance::SpendGold(int32 Amount)
{
    if (Gold < Amount) return false;
    Gold -= Amount; return true;
}

void UFF44GameInstance::AddItem(const FItemInstance& Item)
{
    Stash.Add(Item);
}

bool UFF44GameInstance::RemoveItemById(FName ItemId)
{
    int32 Idx = Stash.IndexOfByPredicate([&](const FItemInstance& I) { return I.ItemId == ItemId; });
    if (Idx != INDEX_NONE) { Stash.RemoveAt(Idx); return true; }
    return false;
}


