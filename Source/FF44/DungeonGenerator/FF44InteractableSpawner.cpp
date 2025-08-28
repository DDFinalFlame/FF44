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

        if (const TSubclassOf<AFF44InteractableActor>* TagClass = TaggedPools.Find(Info.Tag))
        {
            if (*TagClass)
            {
                World->SpawnActor<AFF44InteractableActor>(*TagClass, Info.Transform);
            }

            continue;
        }

        if (NoneChance > 0.f && (Rand.FRand() < NoneChance))
        {
            continue;
        }

        if (DefaultPool.Num() == 0)
        {
            if (!bApplyNoneWhenPoolEmpty)
            {
                continue;
            }

            continue;
        }

        const int32 PickIdx = Rand.RandRange(0, DefaultPool.Num() - 1);
        TSubclassOf<AFF44InteractableActor> Picked = DefaultPool[PickIdx];
        if (!*Picked) { continue; }

        World->SpawnActor<AFF44InteractableActor>(Picked, Info.Transform);
    }

    OnSpawnComplete.Broadcast();
}