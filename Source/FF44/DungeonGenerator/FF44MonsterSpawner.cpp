// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator/FF44MonsterSpawner.h"

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
                World->SpawnActor<APawn>(*Found, Info.Transform);
            }
        }
    }

    OnSpawnComplete.Broadcast();
}