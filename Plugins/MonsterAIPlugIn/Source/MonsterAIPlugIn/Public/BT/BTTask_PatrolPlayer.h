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
	// Blackboard�� ������ ��ġ Ű
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector PatrolLocationKey;

	// ��ġ Ž�� �ݰ�
	UPROPERTY(EditAnywhere, Category = "Patrol")
	float SearchRadius = 1000.0f;
};
