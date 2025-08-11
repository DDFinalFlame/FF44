// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator.h"
#include "RB_DungeonRoom1.h"
#include "RoomBase.h"

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
	ARoomBase* LastestSpawnedRoom = this->GetWorld()->SpawnActor<ARoomBase>(RoomsToBeSpawned[rand() % RoomsToBeSpawned.Num()]);

	USceneComponent* SelectedExitPoint = Exits[rand() % Exits.Num()];

	LastestSpawnedRoom->SetActorLocation(SelectedExitPoint->GetComponentLocation());
	LastestSpawnedRoom->SetActorRotation(SelectedExitPoint->GetComponentRotation());
}

