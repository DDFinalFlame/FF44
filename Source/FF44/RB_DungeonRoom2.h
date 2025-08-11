// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RoomBase.h"
#include "RB_DungeonRoom2.generated.h"


class UArrowComponent;

/**
 * 
 */
UCLASS()
class FF44_API ARB_DungeonRoom2 : public ARoomBase
{
	GENERATED_BODY()

public:
	ARB_DungeonRoom2();

public:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UArrowComponent* Start_Arrow;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UArrowComponent* Exit_Arrow_1;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UArrowComponent* Exit_Arrow_2;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* ClosingWall_1;
	
};
