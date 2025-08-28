// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FF44ExitCap.generated.h"

UCLASS()
class FF44_API AFF44ExitCap : public AActor
{
	GENERATED_BODY()
	
public:	
	AFF44ExitCap();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* ExitCap;

};
