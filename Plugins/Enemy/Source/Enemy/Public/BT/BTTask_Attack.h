// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Attack.generated.h"

class UAbilitySystemComponent;
struct FAbilityEndedData;
class UGameplayAbility;
/**
 * 
 */
UCLASS()
class ENEMY_API UBTTask_Attack : public UBTTaskNode
{
	GENERATED_BODY()
protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttackTag")
    FGameplayTag AbilityTag;

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

private:
    void OnAbilityEnded(const FAbilityEndedData& EndData);

    UBehaviorTreeComponent* CachedOwnerComp = nullptr;
    UAbilitySystemComponent* CachedASC = nullptr;
    FDelegateHandle AbilityEndedDelegateHandle;
};
