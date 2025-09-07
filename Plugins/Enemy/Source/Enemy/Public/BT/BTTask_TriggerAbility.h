// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_TriggerAbility.generated.h"

/**
 * 
 */
UCLASS()
class ENEMY_API UBTTask_TriggerAbility : public UBTTaskNode
{
	GENERATED_BODY()
protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttackTag")
    FGameplayTag AbilityTag;

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
