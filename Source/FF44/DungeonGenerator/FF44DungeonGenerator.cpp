// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator/FF44DungeonGenerator.h"
#include "DungeonGenerator/DungeonBase/FF44StarterRoom.h"
#include "DungeonGenerator/DungeonBase/FF44RoomBase.h"
#include "Kismet/GameplayStatics.h"

AFF44DungeonGenerator::AFF44DungeonGenerator()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFF44DungeonGenerator::BeginPlay()
{
    Super::BeginPlay();

    AFF44StarterRoom* Starter = nullptr;
    SpawnStarterRoom(Starter);
    SpawnPlayerAtStart(Starter);
    SpawnNextRoom();
}

void AFF44DungeonGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFF44DungeonGenerator::SpawnStarterRoom(AFF44StarterRoom*& OutStarter)
{
    OutStarter = nullptr;

    if (!StarterRoomClass)
    {
        return;
    }

    OutStarter = GetWorld()->SpawnActor<AFF44StarterRoom>(StarterRoomClass);
    OutStarter->ExitPoints->GetChildrenComponents(false, Exits);
}

void AFF44DungeonGenerator::SpawnPlayerAtStart(const AFF44StarterRoom* Starter)
{
    if (!Starter)
    {
        return;
    }

    FTransform StartT;
    Starter->GetPlayerStartTransform(StartT);

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (APawn* Pawn = PC->GetPawn())
    {
        Pawn->SetActorLocationAndRotation(StartT.GetLocation(), StartT.GetRotation());
    }
}

void AFF44DungeonGenerator::SpawnNextRoom()
{
    LatestSpawnedRoom = this->GetWorld()->SpawnActor<AFF44RoomBase>(RoomsToBeSpawned[rand() % RoomsToBeSpawned.Num()]);
    SelectedExitPoint = Exits[rand() % Exits.Num()];

    LatestSpawnedRoom->SetActorLocation(SelectedExitPoint->GetComponentLocation());
    //LatestSpawnedRoom->SetActorRotation(SelectedExitPoint->GetComponentRotation());

    FRotator R = SelectedExitPoint->GetComponentRotation();
    R.Yaw += 90.f;
    LatestSpawnedRoom->SetActorRotation(R);
}

