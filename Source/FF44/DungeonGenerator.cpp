// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator.h"
#include "RB_DungeonRoom1.h"
#include "RoomBase.h"
#include "Components/BoxComponent.h"

ADungeonGenerator::ADungeonGenerator()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ADungeonGenerator::BeginPlay()
{
	Super::BeginPlay();

	SpawnStarterRoom();
	SpawnNextRoom();

}

void ADungeonGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADungeonGenerator::SpawnStarterRoom()
{
	ARB_DungeonRoom1* SpawnedStarterRoom = this->GetWorld()->SpawnActor<ARB_DungeonRoom1>(StarterRoom);
	SpawnedStarterRoom->SetActorLocation(this->GetActorLocation());
	SpawnedStarterRoom->ExitPointsFolder->GetChildrenComponents(false, Exits);

}

void ADungeonGenerator::SpawnNextRoom()
{
	bCanSpawn = true;

	LastestSpawnedRoom = this->GetWorld()->SpawnActor<ARoomBase>(RoomsToBeSpawned[rand() % RoomsToBeSpawned.Num()]);

	USceneComponent* SelectedExitPoint = Exits[rand() % Exits.Num()];

	LastestSpawnedRoom->SetActorLocation(SelectedExitPoint->GetComponentLocation());
	LastestSpawnedRoom->SetActorRotation(SelectedExitPoint->GetComponentRotation());

	RemoveOverlappingRooms();

	if (bCanSpawn)
	{
		Exits.Remove(SelectedExitPoint);
		TArray<USceneComponent*> LastestRoomExitPoints;
		LastestSpawnedRoom->ExitPointsFolder->GetChildrenComponents(false, LastestRoomExitPoints);
		Exits.Append(LastestRoomExitPoints);
	}

	RoomAmount = RoomAmount - 1;

	if (RoomAmount > 0)
	{
		SpawnNextRoom();
	}
}

void ADungeonGenerator::RemoveOverlappingRooms()
{
	TArray<USceneComponent*> OverlappedRooms;
	LastestSpawnedRoom->OverlapFolder->GetChildrenComponents(false, OverlappedRooms);

	TArray<UPrimitiveComponent*> OverlappingComponents;

	for (USceneComponent* Element : OverlappedRooms)
	{
		Cast<UBoxComponent>(Element)->GetOverlappingComponents(OverlappingComponents);
	}

	for (UPrimitiveComponent* Element : OverlappingComponents)
	{
		bCanSpawn = false;
		LastestSpawnedRoom->Destroy();
	}
}

