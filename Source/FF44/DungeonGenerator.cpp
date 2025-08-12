// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator.h"
#include "RB_DungeonRoom1.h"
#include "RoomBase.h"
#include "Components/BoxComponent.h"
#include "ClosingWall.h"
#include "Door.h"

ADungeonGenerator::ADungeonGenerator()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ADungeonGenerator::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle ExitsHandle;
	FTimerHandle DoorsHandle;

	SetSeed();

	SpawnStarterRoom();
	SpawnNextRoom();

	GetWorld()->GetTimerManager().SetTimer(ExitsHandle, this, &ADungeonGenerator::CloseUnusedExits, 1.0f, false);
	GetWorld()->GetTimerManager().SetTimer(DoorsHandle, this, &ADungeonGenerator::SpawnDoors, 1.0f, false);

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

	int32 RoomIndex = RandomStream.RandRange(0, RoomsToBeSpawned.Num() - 1);
	LastestSpawnedRoom = this->GetWorld()->SpawnActor<ARoomBase>(RoomsToBeSpawned[RoomIndex]);

	int32 ExitInder = RandomStream.RandRange(0, Exits.Num() - 1);
	USceneComponent* SelectedExitPoint = Exits[ExitInder];

	LastestSpawnedRoom->SetActorLocation(SelectedExitPoint->GetComponentLocation());
	LastestSpawnedRoom->SetActorRotation(SelectedExitPoint->GetComponentRotation());

	Doors.Add(SelectedExitPoint);

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
		RoomAmount = RoomAmount + 1;

		LastestSpawnedRoom->Destroy();
	}
}

void ADungeonGenerator::CloseUnusedExits()
{
	for (USceneComponent* Element : Exits)
	{
		AClosingWall* LastestSpawnedClosingWall = GetWorld()->SpawnActor<AClosingWall>(ClosingWall);

		FVector RelativeOffset(-50.f, 0.f, 250.f);
		FVector WorldOffset = Element->GetComponentRotation().RotateVector(RelativeOffset);

		LastestSpawnedClosingWall->SetActorLocation(Element->GetComponentLocation() + WorldOffset);
		LastestSpawnedClosingWall->SetActorRotation(Element->GetComponentRotation() + FRotator(0.f, 90.f, 0.f));
	}
}

void ADungeonGenerator::SpawnDoors()
{
	for (USceneComponent* Element : Doors)
	{
		ADoor* LastestSpawnedDoor = GetWorld()->SpawnActor<ADoor>(Door);

		FVector RelativeOffset(0.f, 0.f, 250.f);
		FVector WorldOffset = Element->GetComponentRotation().RotateVector(RelativeOffset);

		LastestSpawnedDoor->SetActorLocation(Element->GetComponentLocation() + WorldOffset);
		LastestSpawnedDoor->SetActorRotation(Element->GetComponentRotation() + FRotator(0.f, 90.f, 0.f));
	}
}

void ADungeonGenerator::SetSeed()
{
	int32 Results;
	if (Seed == -1)
	{
		Results = FMath::Rand();
	}
	else
	{
		Results = Seed;
	}

	RandomStream.Initialize(Results);
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("%d"), Results));
}

