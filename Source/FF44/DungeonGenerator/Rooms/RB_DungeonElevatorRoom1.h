// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DungeonGenerator/DungeonBase/RoomBase.h"
#include "RB_DungeonElevatorRoom1.generated.h"

/**
 * 
 */
UCLASS()
class FF44_API ARB_DungeonElevatorRoom1 : public ARoomBase
{
	GENERATED_BODY()

protected:
	ARB_DungeonElevatorRoom1();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public:

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UArrowComponent* Exit_Arrow_1;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UArrowComponent* Exit_Arrow_2;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UArrowComponent* Exit_Arrow_3;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UArrowComponent* Exit_Arrow_4;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Floor_1;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Floor_2;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Floor_3;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Floor_4;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Floor_5;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Floor_6;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Floor_7;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* SecondF_Wall_1;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* SecondF_Wall_2;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* SecondF_Wall_3;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* SecondF_Wall_4;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* SecondF_Wall_5;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* SecondF_Wall_6;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* SecondF_Wall_7;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* SecondF_Wall_8;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Closing_Wall_1;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Closing_Wall_2;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Closing_Wall_3;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Platform;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBoxComponent* PlatformBoxCollision;

public:
	FVector StartLocation;
	FVector EndLocation;
	FVector CurrentLocation;
	
	bool bSouldMove = false;

	float MoveSpeed = 1.0f;

public:
	void GoUp();
	void GoDown();

	
};
