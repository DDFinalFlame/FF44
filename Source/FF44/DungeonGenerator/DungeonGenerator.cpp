// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator.h"
#include "Components/BoxComponent.h"

#include "DungeonGenerator/DungeonBase/RoomBase.h"
#include "DungeonGenerator/DungeonBase/ClosingWall.h"
#include "DungeonGenerator/DungeonBase/Door.h"
#include "DungeonGenerator/DungeonBase/CoinBase.h"
#include "DungeonGenerator/DungeonBase/TreasureChestBase.h"

#include "DungeonGenerator/Rooms/RB_DungeonRoom1.h"
#include "DungeonGenerator/Rooms/RB_BossRoom.h"

ADungeonGenerator::ADungeonGenerator()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ADungeonGenerator::BeginPlay()
{
	Super::BeginPlay();

	SetSeed();

	SpawnStarterRoom();
	SpawnNextRoom();

}

void ADungeonGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bDungeonCompleted == true)
	{
		for (int32 i = 0; i < CoinAmount; ++i)
		{
			SpawnCoins();
		}

		for (int32 i = 0; i < ChestAmount; ++i)
		{
			SpawnChests();
		}

		CloseUnusedExits();
		SpawnDoors();

		SpawnBossRoom();

		bDungeonCompleted = false;
	}
}

void ADungeonGenerator::SpawnStarterRoom()
{
	ARB_DungeonRoom1* SpawnedStarterRoom = this->GetWorld()->SpawnActor<ARB_DungeonRoom1>(StarterRoom);
	SpawnedStarterRoom->SetActorLocation(this->GetActorLocation());
	SpawnedStarterRoom->ExitPointsFolder->GetChildrenComponents(false, Exits);
	SpawnedStarterRoom->FloorSpawnPoints->GetChildrenComponents(false, SpawnPoints);

}

void ADungeonGenerator::SpawnNextRoom()
{
	bCanSpawn = true;

	if (RoomAmount % 10 == 0)
	{
		int32 SpecialRoomIndex = RandomStream.RandRange(0, SpecialRoomsToBeSpawned.Num() - 1);
		LastestSpawnedRoom = this->GetWorld()->SpawnActor<ARoomBase>(SpecialRoomsToBeSpawned[SpecialRoomIndex]);
	}
	else
	{
		int32 RoomIndex = RandomStream.RandRange(0, RoomsToBeSpawned.Num() - 1);
		LastestSpawnedRoom = this->GetWorld()->SpawnActor<ARoomBase>(RoomsToBeSpawned[RoomIndex]);
	}

	int32 ExitInder = RandomStream.RandRange(0, Exits.Num() - 1);
	SelectedExitPoint = Exits[ExitInder];

	LastestSpawnedRoom->SetActorLocation(SelectedExitPoint->GetComponentLocation());
	LastestSpawnedRoom->SetActorRotation(SelectedExitPoint->GetComponentRotation());

	Doors.Add(SelectedExitPoint);

	LastestSpawnedRoom->FloorSpawnPoints->GetChildrenComponents(false, LastestRoomSpawnPoints);
	SpawnPoints.Append(LastestRoomSpawnPoints);

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
		GetWorld()->GetTimerManager().SetTimer(SpawningRoomHandle, this, &ADungeonGenerator::SpawnNextRoom, 0.01f, false);
	}
	else
	{
		bDungeonCompleted = true;
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

void ADungeonGenerator::SpawnCoins()
{
	if (CoinAmount > 0)
	{
		USceneComponent* SelectedSpawnPoint;

		int32 SpawnPointIndex = RandomStream.RandRange(0, SpawnPoints.Num() - 1);
		SelectedSpawnPoint = SpawnPoints[SpawnPointIndex];

		ACoinBase* LastestCoinSpawned = this->GetWorld()->SpawnActor<ACoinBase>(Coin);

		LastestCoinSpawned->SetActorLocation(SelectedSpawnPoint->GetComponentLocation() + FVector(0.f, 0.f, 100.f));

		SpawnPoints.Remove(SelectedSpawnPoint);

		CoinAmount = CoinAmount - 1;
	}
}

void ADungeonGenerator::SpawnChests()
{
	if (ChestAmount > 0)
	{
		USceneComponent* SelectedSpawnPoint;

		int32 SpawnPointIndex = RandomStream.RandRange(0, SpawnPoints.Num() - 1);
		SelectedSpawnPoint = SpawnPoints[SpawnPointIndex];

		ATreasureChestBase* LastestChestSpawned = this->GetWorld()->SpawnActor<ATreasureChestBase>(Chest);

		LastestChestSpawned->SetActorLocation(SelectedSpawnPoint->GetComponentLocation());

		SpawnPoints.Remove(SelectedSpawnPoint);

		ChestAmount = ChestAmount - 1;
	}
}

void ADungeonGenerator::SpawnBossRoom()
{
	ARB_BossRoom* BossRoom = GetWorld()->SpawnActor<ARB_BossRoom>(BossRoomToBeSpawned);

	BossRoom->SetActorLocation(SelectedExitPoint->GetComponentLocation());
	BossRoom->SetActorRotation(SelectedExitPoint->GetComponentRotation());

	LastestSpawnedRoom->Destroy();


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

