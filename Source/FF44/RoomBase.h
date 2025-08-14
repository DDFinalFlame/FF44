// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomBase.generated.h"


class UBoxComponent;
class UArrowComponent;

UCLASS()
class FF44_API ARoomBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ARoomBase();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* DefaultSceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* GeometryFolder;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* FloorSpawnPoints;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* OverlapFolder;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* ExitPointsFolder;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Floor;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Wall_1;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Wall_2;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Wall_3;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Wall_4;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Wall_5;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Wall_6;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Wall_7;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Wall_8;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBoxComponent* BoxCollision;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UArrowComponent* Arrow;

};
