// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator/FF44FloorManager.h"
#include "DungeonGenerator/FF44DungeonGenerator.h"
#include "DungeonGenerator/FF44MonsterSpawner.h"
#include "DungeonGenerator/FF44InteractableSpawner.h"
#include "Kismet/GameplayStatics.h"

AFF44FloorManager::AFF44FloorManager()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AFF44FloorManager::BeginPlay()
{
	Super::BeginPlay();
	StartRun(BaseSeed, 1);
}

void AFF44FloorManager::StartRun(int32 InBaseSeed, int32 StartFloor)
{
    BaseSeed = InBaseSeed;
    CurrentFloor = StartFloor;
    NextFloor();
}

void AFF44FloorManager::NextFloor()
{
    CleanupFloor();
    StartFloorInternal();
}

void AFF44FloorManager::CleanupFloor()
{
    if (Dungeon)
    {
        Dungeon->Destroy();
        Dungeon = nullptr;
    }

    if (MonsterSpawner)
    {
        MonsterSpawner->Destroy();
        MonsterSpawner = nullptr;
    }

    if (InteractableSpawner)
    {
        InteractableSpawner->Destroy();
        InteractableSpawner = nullptr;
    }

    bFloorReady = false;
}

void AFF44FloorManager::StartFloorInternal()
{
    OnFloorStarted.Broadcast(CurrentFloor);

    if (!DungeonGeneratorClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("FloorManager: DungeonGeneratorClass not set."));
        return;
    }

    Dungeon = GetWorld()->SpawnActor<AFF44DungeonGenerator>(DungeonGeneratorClass);
    if (!Dungeon)
    {
        UE_LOG(LogTemp, Warning, TEXT("FloorManager: Failed to spawn DungeonGenerator."));
        return;
    }

    Dungeon->bIsBossFloor = IsBossFloor();

    // (선택) 시드 전달이 필요하면 Dungeon 쪽에 프로퍼티/세터 추가해서 SeedForFloor()를 넘겨주면 됨.
    // Dungeon->GenerationSeed = SeedForFloor();

    Dungeon->OnDungeonComplete.AddDynamic(this, &AFF44FloorManager::HandleDungeonComplete);
}

void AFF44FloorManager::HandleDungeonComplete()
{
    if (!Dungeon) return;

    CachedMonsterMarkers = Dungeon->GetMonsterSpawnMarkers();

    if (MonsterSpawnerClass && !MonsterSpawner)
    {
        MonsterSpawner = GetWorld()->SpawnActor<AFF44MonsterSpawner>(MonsterSpawnerClass);
        if (MonsterSpawner)
        {
            MonsterSpawner->OnSpawnComplete.AddDynamic(this, &AFF44FloorManager::HandleMonsterSpawnComplete);
        }
    }

    if (MonsterSpawner)
    {
        MonsterSpawner->SpawnFromMarkers(CachedMonsterMarkers);
    }
    else
    {
        HandleMonsterSpawnComplete();
    }
}

void AFF44FloorManager::HandleMonsterSpawnComplete()
{
    const TArray<FInteractableSpawnInfo>& InterMarkers = Dungeon->GetInteractableSpawnMarkers();

    if (!InteractableSpawner)
    {
        InteractableSpawner = GetWorld()->SpawnActor<AFF44InteractableSpawner>(InteractableSpawnerClass);
        if (InteractableSpawner)
        {
            InteractableSpawner->OnSpawnComplete.AddDynamic(this, &AFF44FloorManager::HandleInteractableSpawnComplete);
        }
    }

    if (InteractableSpawner)
    {
        InteractableSpawner->SpawnFromMarkers(InterMarkers, SeedForFloor() + 17); // Seed 사용!
    }
    else
    {
        HandleInteractableSpawnComplete();
    }
}

void AFF44FloorManager::HandleInteractableSpawnComplete()
{
    bFloorReady = true;
    OnFloorReady.Broadcast(CurrentFloor);
}
