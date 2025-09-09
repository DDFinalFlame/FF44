// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator/FF44InteractableSpawner.h"
#include "Interactable/FF44InteractableActor.h"

AFF44InteractableSpawner::AFF44InteractableSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFF44InteractableSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearAllTimersForObject(this);
    OnSpawnComplete.Clear();
    Super::EndPlay(EndPlayReason);
}

void AFF44InteractableSpawner::SpawnFromMarkers(const TArray<FInteractableSpawnInfo>& Markers, int32 Seed)
{
    UWorld* World = GetWorld();
    if (!World) { OnSpawnComplete.Broadcast(); return; }

    FRandomStream Rand(Seed);

    for (int32 i = 0; i < Markers.Num(); ++i)
    {
        const FInteractableSpawnInfo& Info = Markers[i];

        AFF44InteractableActor* Spawned = nullptr;

        if (const TSubclassOf<AFF44InteractableActor>* TagClass = TaggedPools.Find(Info.Tag))
        {
            if (*TagClass)
            {
                Spawned = World->SpawnActor<AFF44InteractableActor>(*TagClass, Info.Transform);
            }
        }
        else
        {
            if (!(NoneChance > 0.f && (Rand.FRand() < NoneChance)))
            {
                if (DefaultPool.Num() > 0)
                {
                    const int32 PickIdx = Rand.RandRange(0, DefaultPool.Num() - 1);
                    TSubclassOf<AFF44InteractableActor> Picked = DefaultPool[PickIdx];
                    if (*Picked)
                    {
                        Spawned = World->SpawnActor<AFF44InteractableActor>(Picked, Info.Transform);
                    }
                }
            }
        }

        if (Spawned)
        {
            SpawnedActors.Add(Spawned);
        }
    }

    OnSpawnComplete.Broadcast();
}

void AFF44InteractableSpawner::CleanupSpawned()
{
    for (auto& W : SpawnedActors)
    {
        if (AActor* A = W.Get())
        {
            if (IsValid(A)) A->Destroy();
        }
    }

    SpawnedActors.Empty();
}
