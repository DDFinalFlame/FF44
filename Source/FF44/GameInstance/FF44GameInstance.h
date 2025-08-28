// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "FF44GameInstance.generated.h"

USTRUCT(BlueprintType)
struct FItemInstance
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ItemId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Quantity = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Price = 0;
};

USTRUCT(BlueprintType)
struct FRunConfig
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Seed = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bBossFloor = false;
};

UCLASS()
class FF44_API UFF44GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
    int32 Gold = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<FItemInstance> Stash;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Run")
    FRunConfig PendingRun;

public:
    UFUNCTION(BlueprintCallable)
    void AddGold(int32 Amount);

    UFUNCTION(BlueprintCallable)
    bool SpendGold(int32 Amount);

    UFUNCTION(BlueprintCallable)
    void AddItem(const FItemInstance& Item);

    UFUNCTION(BlueprintCallable)
    bool RemoveItemById(FName ItemId);

	
};
