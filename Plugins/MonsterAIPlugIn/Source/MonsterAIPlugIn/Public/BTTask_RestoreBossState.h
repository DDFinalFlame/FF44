// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_RestoreBossState.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class MONSTERAIPLUGIN_API UBTTask_RestoreBossState : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
    UBTTask_RestoreBossState();

    // 블랙보드 키 지정 (기본: "BossState", "PrevBossState")
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    struct FBlackboardKeySelector BossStateKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    struct FBlackboardKeySelector PrevBossStateKey;

    // 복귀 후 Prev를 비울지 여부(선택)
    UPROPERTY(EditAnywhere, Category = "Options")
    bool bClearPrevAfterRestore = false;

protected:
    virtual EBTNodeResult::Type ExecuteTask(class UBehaviorTreeComponent& _OwnerComp,
        uint8* _NodeMemory) override;

    virtual void InitializeFromAsset(UBehaviorTree& Asset) override;



};
