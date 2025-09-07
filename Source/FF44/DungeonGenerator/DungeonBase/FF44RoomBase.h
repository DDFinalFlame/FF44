// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FF44RoomBase.generated.h"

class UBoxComponent;

UCLASS()
class FF44_API AFF44RoomBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AFF44RoomBase();

public:
    UPROPERTY(VisibleAnywhere, Category = "Room|Folders")
    USceneComponent* Floors;

    UPROPERTY(VisibleAnywhere, Category = "Room|Folders")
    USceneComponent* Walls;

    UPROPERTY(VisibleAnywhere, Category = "Room|Folders")
    USceneComponent* Environments;

    UPROPERTY(VisibleAnywhere, Category = "Room|Folders")
    USceneComponent* InteractableSpawnPoints;

    UPROPERTY(VisibleAnywhere, Category = "Room|Folders")
    USceneComponent* MonsterSpawnPoints;

    UPROPERTY(VisibleAnywhere, Category = "Room|Folders")
    USceneComponent* ExitPoints;

    UPROPERTY(VisibleAnywhere, Category = "Room|Folders")
    USceneComponent* OverlapFolder;

public:
    UPROPERTY(VisibleAnywhere, Category = "Room|Collision")
    UBoxComponent* Bounds;

    UPROPERTY(EditDefaultsOnly, Category = "Room|Meta")
    FName RoomTypeTag;

    UPROPERTY(EditDefaultsOnly, Category = "Room|Meta")
    int32 SpawnWeight = 1;

public:
    UFUNCTION(BlueprintCallable, Category = "Room")
    void GetFloorMeshes(TArray<USceneComponent*>& OutFloors) const;

    UFUNCTION(BlueprintCallable, Category = "Room")
    void GetWallMeshes(TArray<USceneComponent*>& OutWalls) const;

    UFUNCTION(BlueprintCallable, Category = "Room")
    void GetExitComponents(TArray<USceneComponent*>& OutExits) const;

};
