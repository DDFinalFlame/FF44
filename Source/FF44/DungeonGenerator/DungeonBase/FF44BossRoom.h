// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DungeonGenerator/DungeonBase/FF44RoomBase.h"
#include "FF44BossRoom.generated.h"

/**
 * 
 */
UCLASS()
class FF44_API AFF44BossRoom : public AFF44RoomBase
{
	GENERATED_BODY()

public:
	AFF44BossRoom();

public:
	UPROPERTY(VisibleAnywhere, Category = "Room|Folders")
	USceneComponent* PortalPoints;

	UPROPERTY(VisibleAnywhere, Category = "Room|Folders")
	USceneComponent* BossPoints;
	
};
