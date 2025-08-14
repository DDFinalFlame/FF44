// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RoomBase.h"
#include "RB_BossRoom.generated.h"

/**
 * 
 */
UCLASS()
class FF44_API ARB_BossRoom : public ARoomBase
{
	GENERATED_BODY()

public:
	ARB_BossRoom();
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Closing_Wall_1;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Closing_Wall_2;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Closing_Wall_3;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* RedPlane;

};
