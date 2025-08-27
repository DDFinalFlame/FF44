// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterSpawnInfo.h"
#include "FF44DungeonGenerator.generated.h"

class AFF44StarterRoom;
class AFF44RoomBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDungeonComplete);

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

	UPROPERTY(EditAnywhere, Category = "Rooms|Special")
	TSubclassOf<AFF44RoomBase> PortalRoomClass;

	UPROPERTY(EditAnywhere, Category = "Rooms|Special")
	TSubclassOf<AFF44RoomBase> BossRoomClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor")
	bool bIsBossFloor = false;

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

	UPROPERTY(BlueprintReadOnly, Category = "Dungeon|Markers")
	TArray<FMonsterSpawnInfo> MonsterSpawnMarkers;

	UPROPERTY(BlueprintReadOnly, Category = "Dungeon|Markers")
	TArray<FInteractableSpawnInfo> InteractableSpawnMarkers;

	UFUNCTION(BlueprintCallable, Category = "Dungeon|Markers")
	const TArray<FMonsterSpawnInfo>& GetMonsterSpawnMarkers() const { return MonsterSpawnMarkers; }

	UFUNCTION(BlueprintCallable, Category = "Dungeon|Markers")
	const TArray<FInteractableSpawnInfo>& GetInteractableSpawnMarkers() const { return InteractableSpawnMarkers; }

	UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
	bool bDungeonCompleted = false;

	UPROPERTY(BlueprintAssignable, Category = "Dungeon")
	FOnDungeonComplete OnDungeonComplete;

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

	USceneComponent* SelectGoalExit() const;
	void PlaceFloorGoalAndFinish();
	bool IsRoomOverlapping(AFF44RoomBase* Room) const;

	void CollectMonsterMarkersFromRoom(const AFF44RoomBase* Room);
	void CollectInteractableMarkersFromRoom(const AFF44RoomBase* Room);

	TSubclassOf<AFF44RoomBase> PickWeightedRoom(const TArray<TSubclassOf<AFF44RoomBase>>& Pool) const;
};
