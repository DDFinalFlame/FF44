// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnInfo.h"
#include "FF44InteractableSpawner.generated.h"

class AFF44InteractableActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInteractableSpawnComplete);

UCLASS()
class FF44_API AFF44InteractableSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
    AFF44InteractableSpawner();

protected:
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    UPROPERTY(BlueprintAssignable, Category = "Spawner")
    FInteractableSpawnComplete OnSpawnComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Pools")
    TArray<TSubclassOf<AFF44InteractableActor>> DefaultPool;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Pools")
    TMap<FName, TSubclassOf<AFF44InteractableActor>> TaggedPools;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Rules", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float NoneChance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Rules")
    bool bApplyNoneWhenPoolEmpty = false;

    UPROPERTY(Transient)
    TArray<TWeakObjectPtr<AActor>> SpawnedActors;

    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void SpawnFromMarkers(const TArray<FInteractableSpawnInfo>& Markers, int32 Seed);

    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void CleanupSpawned();
};
