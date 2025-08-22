// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CombatEventData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class UCombatEventData : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    float AttackPower;

    UPROPERTY(BlueprintReadWrite)
    float Defense;
};
