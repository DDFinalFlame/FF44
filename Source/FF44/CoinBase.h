// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoinBase.generated.h"


class USphereComponent;
class URotatingMovementComponent;

UCLASS()
class FF44_API ACoinBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ACoinBase();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Coin;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* CoinCollision;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	URotatingMovementComponent* RotatingMovementComponent;

};
