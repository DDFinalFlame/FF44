// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator/FF44MonsterSpawner.h"

void AFF44MonsterSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    CleanupSpawned();
    GetWorldTimerManager().ClearAllTimersForObject(this);
    OnSpawnComplete.Clear();
    Super::EndPlay(EndPlayReason);
}

void AFF44MonsterSpawner::SpawnFromMarkers(const TArray<FMonsterSpawnInfo>& Markers)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        OnSpawnComplete.Broadcast();
        return;
    }

    for (const FMonsterSpawnInfo& Info : Markers)
    {
        if (const TSubclassOf<APawn>* Found = MonsterMap.Find(Info.Tag))
        {
            if (*Found)
            {
                APawn* Spawned = World->SpawnActor<APawn>(*Found, Info.Transform);
                if (Spawned)
                {
                    SpawnedActors.Add(Spawned);
                }
            }
        }
    }

    OnSpawnComplete.Broadcast();
}

void AFF44MonsterSpawner::CleanupSpawned()
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