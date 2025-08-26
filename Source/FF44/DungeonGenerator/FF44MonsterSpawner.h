// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterSpawnInfo.h"
#include "FF44MonsterSpawner.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSpawnComplete);

UCLASS()
class FF44_API AFF44MonsterSpawner : public AActor
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "Spawner")
    FSpawnComplete OnSpawnComplete;

    UPROPERTY(EditAnywhere, Category = "Spawner")
    TMap<FName, TSubclassOf<APawn>> MonsterMap;

    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void SpawnFromMarkers(const TArray<FMonsterSpawnInfo>& Markers /*, int32 Seed = 0*/);
};
