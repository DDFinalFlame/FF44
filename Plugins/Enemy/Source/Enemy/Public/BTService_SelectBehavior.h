// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "EnemyDefine.h"
#include "BTService_SelectBehavior.generated.h"

class ABaseEnemy;
/**
 * 
 */
UCLASS()
class ENEMY_API UBTService_SelectBehavior : public UBTService
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	ABaseEnemy* ControlledEnemy;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector TargetKey;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector NoiseTargetKey;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector BehaviorKey;

	UPROPERTY(EditAnywhere)
	float AttackRangeDistance = 500.0f;
public:
	UBTService_SelectBehavior();

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	void SetBehaviorKey(UBlackboardComponent* BlackboardComponent, EAIBehavior Behavior) const;
	void UpdateBehavior(UBlackboardComponent* BlackboardComponent);
};
