// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SetMonsterState.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class MONSTERAIPLUGIN_API UBTTask_SetMonsterState : public UBTTaskNode
{
	GENERATED_BODY()
public:
    UBTTask_SetMonsterState();

    // �������Ʈ���� �ٷ� ���ڸ� �ٲ� ����
    UPROPERTY(EditAnywhere, Category = "State")
    int32 TargetState = 3; // CombatReady=3

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};