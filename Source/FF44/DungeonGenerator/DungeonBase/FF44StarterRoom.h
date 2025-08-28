// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DungeonGenerator/DungeonBase/FF44RoomBase.h"
#include "FF44StarterRoom.generated.h"

class UArrowComponent;

/**
 * 
 */
UCLASS()
class FF44_API AFF44StarterRoom : public AFF44RoomBase
{
	GENERATED_BODY()

public:
	AFF44StarterRoom();

public:
	UPROPERTY(VisibleAnywhere, Category = "Room|Folders")
	USceneComponent* PlayerStart;

	UPROPERTY(VisibleAnywhere, Category = "StarterRoom")
	UArrowComponent* PlayerStartPoint;

public:
	UFUNCTION(BlueprintCallable, Category = "StarterRoom")
	bool GetPlayerStartTransform(FTransform& OutTransform) const;

};
