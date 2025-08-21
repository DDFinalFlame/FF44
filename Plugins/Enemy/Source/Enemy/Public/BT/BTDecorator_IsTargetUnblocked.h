// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_IsTargetUnblocked.generated.h"

/**
 * 
 */
UCLASS()
class ENEMY_API UBTDecorator_IsTargetUnblocked : public UBTDecorator
{
	GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere)
    FBlackboardKeySelector TargetKey;

protected:
    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

private:
    bool IsTargetUnblocked(APawn* Observer, AActor* Target) const;
	
};
