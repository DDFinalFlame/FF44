// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonGenerator.generated.h"


class ARB_DungeonRoom1;
class ARoomBase;

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

public:
	UPROPERTY(EditAnywhere, Category = "Rooms")
	TSubclassOf<ARB_DungeonRoom1> StarterRoom;

	UPROPERTY(EditAnywhere, Category = "Rooms")
	TArray<TSubclassOf<ARoomBase>> RoomsToBeSpawned;

	TArray<USceneComponent*> Exits;

	UPROPERTY(EditAnywhere, Category = "Dungeon Info")
	int32 RoomAmount;

	ARoomBase* LastestSpawnedRoom;

	bool bCanSpawn;

};
