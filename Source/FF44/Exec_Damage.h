// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "Exec_Damage.generated.h"

/**
 * 
 */
UCLASS()
class FF44_API UExec_Damage : public UGameplayEffectExecutionCalculation
{
    GENERATED_BODY()
public:
    UExec_Damage();

    UPROPERTY()
    FGameplayEffectAttributeCaptureDefinition Attack;

    virtual void Execute_Implementation(
        const FGameplayEffectCustomExecutionParameters& Params,
        FGameplayEffectCustomExecutionOutput& Out) const override;
	
};
