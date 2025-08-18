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

public:
	AFF44RoomBase* LatestSpawnedRoom;
	USceneComponent* SelectedExitPoint;

	TArray<USceneComponent*> LastestRoomSpawnPoints;
	TArray<USceneComponent*> Exits;

private:
	void SpawnStarterRoom(AFF44StarterRoom*& OutStarter);
	void SpawnPlayerAtStart(const AFF44StarterRoom* Starter);
	void SpawnNextRoom();
};
