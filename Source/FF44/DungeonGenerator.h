// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonGenerator.generated.h"


class ARB_DungeonRoom1;
class ARoomBase;
class AClosingWall;
class ADoor;
class ACoinBase;
class ATreasureChestBase;

UCLASS()
class FF44_API ADungeonGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	ADungeonGenerator();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

public:
	void SpawnStarterRoom();
	void SpawnNextRoom();
	void RemoveOverlappingRooms();
	void CloseUnusedExits();
	void SpawnDoors();
	void SpawnCoins();
	void SpawnChests();
	void SetSeed();

public:
	UPROPERTY(EditAnywhere, Category = "Rooms")
	TSubclassOf<ARB_DungeonRoom1> StarterRoom;

	UPROPERTY(EditAnywhere, Category = "Rooms")
	TArray<TSubclassOf<ARoomBase>> RoomsToBeSpawned;

	UPROPERTY(EditAnywhere, Category = "Rooms")
	TArray<TSubclassOf<ARoomBase>> SpecialRoomsToBeSpawned;

	UPROPERTY(EditAnywhere, Category = "Unused Exits Closing Wall")
	TSubclassOf<AClosingWall> ClosingWall;

	UPROPERTY(EditAnywhere, Category = "Door")
	TSubclassOf<ADoor> Door;

	UPROPERTY(EditAnywhere, Category = "Coin")
	TSubclassOf<ACoinBase> Coin;

	UPROPERTY(EditAnywhere, Category = "Chest")
	TSubclassOf<ATreasureChestBase> Chest;

	UPROPERTY(EditAnywhere, Category = "Dungeon Info")
	int32 RoomAmount;

	UPROPERTY(EditAnywhere, Category = "Dungeon Info")
	int32 CoinAmount;

	UPROPERTY(EditAnywhere, Category = "Dungeon Info")
	int32 ChestAmount;

	UPROPERTY(EditAnywhere, Category = "Dungeon Info")
	int32 Seed;

	ARoomBase* LastestSpawnedRoom;

	TArray<USceneComponent*> Exits;
	TArray<USceneComponent*> Doors;
	TArray<USceneComponent*> SpawnPoints;
	TArray<USceneComponent*> LastestRoomSpawnPoints;

	bool bCanSpawn;

	FRandomStream RandomStream;

};
