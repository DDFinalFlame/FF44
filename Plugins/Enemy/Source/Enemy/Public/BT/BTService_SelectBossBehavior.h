// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyDefine.h"
#include "BehaviorTree/BTService.h"
#include "BTService_SelectBossBehavior.generated.h"

class ABaseEnemy;
/**
 * 
 */
UCLASS()
class ENEMY_API UBTService_SelectBossBehavior : public UBTService
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector TargetKey;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector PhaseKey;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector BehaviorKey;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector bOpeningPatternDoneKey;

public:
	UBTService_SelectBossBehavior();

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	void SetBehaviorKey(UBlackboardComponent* BlackboardComponent, EAIBehavior Behavior) const;
	void UpdateBehavior(UBlackboardComponent* BlackboardComponent, ABaseEnemy* ControlledEnemy);
};
