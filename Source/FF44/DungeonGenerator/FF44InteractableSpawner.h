// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterSpawnInfo.h"
#include "FF44InteractableSpawner.generated.h"

class AFF44InteractableActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInteractableSpawnComplete);

UCLASS()
class FF44_API AFF44InteractableSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
    AFF44InteractableSpawner();

public:
    UPROPERTY(BlueprintAssignable, Category = "Spawner")
    FInteractableSpawnComplete OnSpawnComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Pools")
    TArray<TSubclassOf<AFF44InteractableActor>> DefaultPool;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Pools")
    TMap<FName, TArray<TSubclassOf<AFF44InteractableActor>>> TaggedPools;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Rules", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float NoneChance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Rules")
    bool bApplyNoneWhenPoolEmpty = false;

    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void SpawnFromMarkers(const TArray<FInteractableSpawnInfo>& Markers, int32 Seed);

};
