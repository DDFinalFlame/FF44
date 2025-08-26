// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator/FF44FloorManager.h"
#include "DungeonGenerator/FF44DungeonGenerator.h"
#include "DungeonGenerator/FF44MonsterSpawner.h"
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

    bFloorReady = false;
}

void AFF44FloorManager::StartFloorInternal()
{
    OnFloorStarted.Broadcast(CurrentFloor);

    if (!DungeonGenClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("FloorManager: DungeonGenClass not set."));
        return;
    }

    Dungeon = GetWorld()->SpawnActor<AFF44DungeonGenerator>(DungeonGenClass);
    if (!Dungeon)
    {
        UE_LOG(LogTemp, Warning, TEXT("FloorManager: Failed to spawn DungeonGenerator."));
        return;
    }

    Dungeon->bIsBossFloor = IsBossFloor();

    // (����) �õ� ������ �ʿ��ϸ� Dungeon �ʿ� ������Ƽ/���� �߰��ؼ� SeedForFloor()�� �Ѱ��ָ� ��.
    // Dungeon->GenerationSeed = SeedForFloor();

    Dungeon->OnDungeonComplete.AddDynamic(this, &AFF44FloorManager::HandleDungeonComplete);
}

void AFF44FloorManager::HandleDungeonComplete()
{
    if (!Dungeon) return;

    CachedMonsterMarkers = Dungeon->GetMonsterSpawnMarkers();

    if (!MonsterSpawnerClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("FloorManager: MonsterSpawnerClass not set."));
        
        // �����ʰ� ��� ������ �����ϰ� �ʹٸ� �Ʒ� �� �� ����
        bFloorReady = true;
        OnFloorReady.Broadcast(CurrentFloor);
        return;
    }

    if (!MonsterSpawner)
    {
        MonsterSpawner = GetWorld()->SpawnActor<AFF44MonsterSpawner>(MonsterSpawnerClass);
        if (!MonsterSpawner)
        {
            UE_LOG(LogTemp, Warning, TEXT("FloorManager: Failed to spawn MonsterSpawner."));
            bFloorReady = true;
            OnFloorReady.Broadcast(CurrentFloor);
            return;
        }
        MonsterSpawner->OnSpawnComplete.AddDynamic(this, &AFF44FloorManager::HandleMonsterSpawnComplete);
    }

    MonsterSpawner->SpawnFromMarkers(CachedMonsterMarkers /*, SeedForFloor()*/);
}

void AFF44FloorManager::HandleMonsterSpawnComplete()
{
    bFloorReady = true;
    OnFloorReady.Broadcast(CurrentFloor);
}
