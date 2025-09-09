// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnInfo.h"
#include "FF44MonsterSpawner.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSpawnComplete);

UCLASS()
class FF44_API AFF44MonsterSpawner : public AActor
{
    GENERATED_BODY()

protected:
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    UPROPERTY(BlueprintAssignable, Category = "Spawner")
    FSpawnComplete OnSpawnComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    TMap<FName, TSubclassOf<APawn>> MonsterMap;

    UPROPERTY(Transient)
    TArray<TWeakObjectPtr<AActor>> SpawnedActors;

    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void SpawnFromMarkers(const TArray<FMonsterSpawnInfo>& Markers);

    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void CleanupSpawned();
};
