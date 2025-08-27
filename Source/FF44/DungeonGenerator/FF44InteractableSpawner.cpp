// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator/FF44InteractableSpawner.h"
#include "Interactable/FF44InteractableActor.h"

AFF44InteractableSpawner::AFF44InteractableSpawner()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AFF44InteractableSpawner::SpawnFromMarkers(const TArray<FInteractableSpawnInfo>& Markers, int32 Seed)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        OnSpawnComplete.Broadcast();
        return;
    }

    FRandomStream Rand(Seed);

    for (int32 i = 0; i < Markers.Num(); ++i)
    {
        const FInteractableSpawnInfo& Info = Markers[i];

        const TArray<TSubclassOf<AFF44InteractableActor>>* Pool = TaggedPools.Find(Info.Tag);
        if (!Pool) Pool = &DefaultPool;

        const bool bRollNone = (NoneChance > 0.f) && (Rand.FRand() < NoneChance);
        if (bRollNone && (bApplyNoneWhenPoolEmpty || (Pool->Num() > 0)))
        {
            continue;
        }

        if (Pool->Num() == 0)
        {
            continue;
        }

        const int32 Idx = Rand.RandRange(0, Pool->Num() - 1);
        TSubclassOf<AFF44InteractableActor> Picked = (*Pool)[Idx];
        if (!*Picked)
        {
            continue;
        }

        World->SpawnActor<AFF44InteractableActor>(Picked, Info.Transform);
    }

    OnSpawnComplete.Broadcast();
}