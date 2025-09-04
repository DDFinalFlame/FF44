// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PatrolPlayer.generated.h"

UCLASS(Blueprintable)
class MONSTERAIPLUGIN_API UBTTask_PatrolPlayer : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_PatrolPlayer();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

public:
	// Blackboard에 저장할 위치 키
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector PatrolLocationKey;

	// 위치 탐색 반경
	UPROPERTY(EditAnywhere, Category = "Patrol")
	float SearchRadius = 1000.0f;
};
