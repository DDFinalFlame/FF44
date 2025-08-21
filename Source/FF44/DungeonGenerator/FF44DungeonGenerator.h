// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FF44DungeonGenerator.generated.h"

class AFF44StarterRoom;
class AFF44RoomBase;

UCLASS()
class FF44_API AFF44DungeonGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	AFF44DungeonGenerator();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(EditAnywhere, Category = "Rooms|Setup")
    TSubclassOf<AFF44StarterRoom> StarterRoomClass;

    UPROPERTY(EditAnywhere, Category = "Rooms|Pools")
    TArray<TSubclassOf<AFF44RoomBase>> RoomsToBeSpawned;

    UPROPERTY(EditAnywhere, Category = "Rooms|Pools")
    TArray<TSubclassOf<AFF44RoomBase>> SmallRoomsToBeSpawned;

    UPROPERTY(EditAnywhere, Category = "Rooms|Control")
    int32 RoomsToSpawn = 30;

    UPROPERTY(EditAnywhere, Category = "Rooms|Control", meta = (ClampMin = "1"))
    int32 MaxTotalRooms = 100;

	UPROPERTY(EditAnywhere, Category = "Rooms|Control")
	float RoomSpawnInterval = 0.01f;

    UPROPERTY(EditAnywhere, Category = "Rooms|Seal")
    TSubclassOf<AActor> ExitCapClass;

	UPROPERTY(EditAnywhere, Category = "Rooms|Seal")
	TSubclassOf<AActor> SmallExitCapClass;

	UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
	bool bDungeonCompleted = false;

public:
	TArray<USceneComponent*> Exits;
	TArray<FName> RecentTwoRoomTags;

	UPROPERTY(Transient)
	AFF44StarterRoom* StarterRoomRef = nullptr;

	UPROPERTY(Transient)
	USceneComponent* SelectedExitPoint = nullptr;

	UPROPERTY(Transient)
	AFF44RoomBase* LatestSpawnedRoom = nullptr;

	FTimerHandle SpawnNextHandle;

	int32 TotalSpawned = 0;


private:
	void SpawnStarterRoom(AFF44StarterRoom*& OutStarter);
	void SpawnPlayerAtStart(const AFF44StarterRoom* Starter);

	void SpawnNextRoom();
	bool RemoveOverlappingRooms();
	void SealRemainingExits();

	TSubclassOf<AFF44RoomBase> PickWeightedRoom(const TArray<TSubclassOf<AFF44RoomBase>>& Pool) const;
};
