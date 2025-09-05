// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator/FF44FloorManager.h"
#include "DungeonGenerator/FF44DungeonGenerator.h"
#include "DungeonGenerator/FF44MonsterSpawner.h"
#include "DungeonGenerator/FF44InteractableSpawner.h"
#include "Interactable/FF44Portal.h"
#include "Kismet/GameplayStatics.h"
#include "DungeonGenerator/DungeonBase/FF44RoomBase.h"

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

void AFF44FloorManager::StartFloorInternal()
{
    OnFloorStarted.Broadcast(CurrentFloor);

    if (!DungeonGeneratorClass) return;

    Dungeon = GetWorld()->SpawnActor<AFF44DungeonGenerator>(DungeonGeneratorClass);
    if (!Dungeon) return;

    Dungeon->bIsBossFloor = IsBossFloor();
    // 필요한 경우: Dungeon->GenerationSeed = SeedForFloor();

    Dungeon->OnDungeonComplete.AddDynamic(this, &AFF44FloorManager::HandleDungeonComplete);
}

void AFF44FloorManager::CleanupFloor()
{
    UnbindFromPortals();

    if (MonsterSpawner)
    {
        MonsterSpawner->CleanupSpawned();
        MonsterSpawner->Destroy();
        MonsterSpawner = nullptr;
    }

    if (InteractableSpawner)
    {
        InteractableSpawner->CleanupSpawned();
        InteractableSpawner->Destroy();
        InteractableSpawner = nullptr;
    }

    if (Dungeon)
    {
        Dungeon->ClearDungeonContents();
        Dungeon->Destroy();
        Dungeon = nullptr;
    }

    bFloorReady = false;
    bMonstersDone = false;
    bInteractablesDone = false;

    CachedMonsterMarkers.Reset();
    CachedInteractableMarkers.Reset();
}

void AFF44FloorManager::TryFinishFloorReady()
{
    if (bMonstersDone && bInteractablesDone && !bFloorReady)
    {
        bFloorReady = true;
        OnFloorReady.Broadcast(CurrentFloor);
    }
}

void AFF44FloorManager::HandleDungeonComplete()
{
    if (!Dungeon) return;

    CachedMonsterMarkers = Dungeon->GetMonsterSpawnMarkers();
    CachedInteractableMarkers = Dungeon->GetInteractableSpawnMarkers();

    if (MonsterSpawnerClass && !MonsterSpawner)
    {
        MonsterSpawner = GetWorld()->SpawnActor<AFF44MonsterSpawner>(MonsterSpawnerClass);
        if (MonsterSpawner)
        {
            MonsterSpawner->OnSpawnComplete.AddDynamic(this, &AFF44FloorManager::HandleMonsterSpawnComplete);
        }
    }
    if (MonsterSpawner && CachedMonsterMarkers.Num() > 0)
    {
        MonsterSpawner->SpawnFromMarkers(CachedMonsterMarkers);
    }
    else
    {
        bMonstersDone = true;
    }

    if (InteractableSpawnerClass && !InteractableSpawner)
    {
        InteractableSpawner = GetWorld()->SpawnActor<AFF44InteractableSpawner>(InteractableSpawnerClass);
        if (InteractableSpawner)
        {
            InteractableSpawner->OnSpawnComplete.AddDynamic(this, &AFF44FloorManager::HandleInteractableSpawnComplete);
        }
    }
    if (InteractableSpawner && CachedInteractableMarkers.Num() > 0)
    {
        InteractableSpawner->SpawnFromMarkers(CachedInteractableMarkers, SeedForFloor() + 17);
    }
    else
    {
        bInteractablesDone = true;
    }

    TryFinishFloorReady();
}

void AFF44FloorManager::HandleMonsterSpawnComplete()
{
    bMonstersDone = true;

    TryFinishFloorReady();
}

void AFF44FloorManager::HandleInteractableSpawnComplete()
{
    bInteractablesDone = true;
    BindToPortals();

    TryFinishFloorReady();
}

void AFF44FloorManager::BindToPortals()
{
    UnbindFromPortals();

    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFF44Portal::StaticClass(), Found);

    for (AActor* A : Found)
    {
        if (AFF44Portal* P = Cast<AFF44Portal>(A))
        {
            P->OnPortalInteracted.AddDynamic(this, &AFF44FloorManager::HandlePortalInteracted);
            BoundPortals.Add(P);
        }
    }
}

void AFF44FloorManager::UnbindFromPortals()
{
    for (TWeakObjectPtr<AFF44Portal>& W : BoundPortals)
    {
        if (AFF44Portal* P = W.Get())
        {
            P->OnPortalInteracted.RemoveDynamic(this, &AFF44FloorManager::HandlePortalInteracted);
        }
    }

    BoundPortals.Empty();
}

void AFF44FloorManager::HandlePortalInteracted(AFF44Portal* Portal, FName PortalTag)
{
    if (PortalTag.IsNone() || PortalTag == TEXT("PortalNext"))
    {
        OnFloorEnded.Broadcast(CurrentFloor);
        CurrentFloor = FMath::Max(1, CurrentFloor + 1);
        NextFloor();
        return;
    }

    if (PortalTag == TEXT("PortalBoss"))
    {
        if (Dungeon)
        {
            if (!IsBossFloor()) { return; }

            MonsterSpawner->CleanupSpawned();
            InteractableSpawner->CleanupSpawned();

            FName FnName = TEXT("EnterBossArena");
            if (Dungeon->GetClass()->FindFunctionByName(FnName))
            {
                Dungeon->CallFunctionByNameWithArguments(*FnName.ToString(), *GLog, nullptr, true);
            }
        }
        return;
    }
}