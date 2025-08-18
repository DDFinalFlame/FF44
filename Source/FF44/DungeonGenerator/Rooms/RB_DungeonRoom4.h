// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DungeonGenerator/DungeonBase/RoomBase.h"
#include "RB_DungeonRoom4.generated.h"


class UArrowComponent;

/**
 * 
 */
UCLASS()
class FF44_API ARB_DungeonRoom4 : public ARoomBase
{
	GENERATED_BODY()

public:
	ARB_DungeonRoom4();

public:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UArrowComponent* Exit_Arrow_1;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* ClosingWall_1;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* ClosingWall_2;
	
};
