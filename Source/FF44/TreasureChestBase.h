// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TreasureChestBase.generated.h"


class UBoxComponent;

UCLASS()
class FF44_API ATreasureChestBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ATreasureChestBase();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Chest;

};
