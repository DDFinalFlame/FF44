// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator/DungeonBase/FF44RoomBase.h"
#include "Components/BoxComponent.h"

AFF44RoomBase::AFF44RoomBase()
{
    PrimaryActorTick.bCanEverTick = false;

    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    Floors = CreateDefaultSubobject<USceneComponent>(TEXT("Floors"));
    Floors->SetupAttachment(RootComponent);

    Walls = CreateDefaultSubobject<USceneComponent>(TEXT("Walls"));
    Walls->SetupAttachment(RootComponent);

    Environments = CreateDefaultSubobject<USceneComponent>(TEXT("Environments"));
    Environments->SetupAttachment(RootComponent);

    InteractableSpawnPoints = CreateDefaultSubobject<USceneComponent>(TEXT("InteractableSpawnPoints"));
    InteractableSpawnPoints->SetupAttachment(RootComponent);

    MonsterSpawnPoints = CreateDefaultSubobject<USceneComponent>(TEXT("MonsterSpawnPoints"));
    MonsterSpawnPoints->SetupAttachment(RootComponent);

    ExitPoints = CreateDefaultSubobject<USceneComponent>(TEXT("ExitPoints"));
    ExitPoints->SetupAttachment(RootComponent);

    OverlapFolder = CreateDefaultSubobject<USceneComponent>(TEXT("OverlapFolder"));
    OverlapFolder->SetupAttachment(RootComponent);

    Bounds = CreateDefaultSubobject<UBoxComponent>(TEXT("Bounds"));
    Bounds->SetupAttachment(OverlapFolder);
    Bounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Bounds->SetCollisionObjectType(ECC_GameTraceChannel2);
    Bounds->SetCollisionResponseToAllChannels(ECR_Ignore);
    Bounds->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap);
    Bounds->SetGenerateOverlapEvents(true);

}

void AFF44RoomBase::GetFloorMeshes(TArray<USceneComponent*>& OutFloors) const
{
    OutFloors.Reset();
    if (!Floors)
    {
        return;
    }

    TArray<USceneComponent*> LocalChildren;
    Floors->GetChildrenComponents(false, LocalChildren);

    OutFloors = MoveTemp(LocalChildren);
}

void AFF44RoomBase::GetWallMeshes(TArray<USceneComponent*>& OutWalls) const
{
    OutWalls.Reset();
    if (!Walls)
    {
        return;
    }

    TArray<USceneComponent*> LocalChildren;
    Walls->GetChildrenComponents(false, LocalChildren);

    OutWalls = MoveTemp(LocalChildren);
}

void AFF44RoomBase::GetExitComponents(TArray<USceneComponent*>& OutExits) const
{
    OutExits.Reset();
    if (!ExitPoints)
    {
        return;
    }

    TArray<USceneComponent*> LocalChildren;
    ExitPoints->GetChildrenComponents(false, LocalChildren);

    OutExits = MoveTemp(LocalChildren);
}