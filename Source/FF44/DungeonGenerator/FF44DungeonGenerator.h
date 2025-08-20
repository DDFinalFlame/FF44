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
    UPROPERTY(EditAnywhere, Category = "Generator|Setup")
    TSubclassOf<AFF44StarterRoom> StarterRoomClass;

	UPROPERTY(EditAnywhere, Category = "Rooms")
	TArray<TSubclassOf<AFF44RoomBase>> RoomsToBeSpawned;

	UPROPERTY(EditAnywhere, Category = "Rooms")
	TArray<TSubclassOf<AFF44RoomBase>> SmallRoomsToBeSpawned;

	UPROPERTY(EditAnywhere, Category = "Rooms")
	int32 RoomsToSpawn = 0;

	UPROPERTY(EditAnywhere, Category = "Rooms")
	TSubclassOf<AActor> ExitCapClass;

public:
	AFF44RoomBase* LatestSpawnedRoom;

	USceneComponent* SelectedExitPoint;

	TArray<USceneComponent*> Exits;
	TArray<USceneComponent*> SmallExits;

	FTimerHandle SpawnNextHandle;

private:
	void SpawnStarterRoom(AFF44StarterRoom*& OutStarter);
	void SpawnPlayerAtStart(const AFF44StarterRoom* Starter);
	void SpawnNextRoom();
	bool RemoveOverlappingRooms();
	void SealRemainingExits();
	TSubclassOf<AFF44RoomBase> PickWeightedRoom(const TArray<TSubclassOf<AFF44RoomBase>>& Pool) const;
};
