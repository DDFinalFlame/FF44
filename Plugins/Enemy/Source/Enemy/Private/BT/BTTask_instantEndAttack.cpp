// Fill out your copyright notice in the Description page of Project Settings.


#include "BT/BTTask_instantEndAttack.h"

#include "AIController.h"
#include "BaseEnemy.h"

EBTNodeResult::Type UBTTask_instantEndAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	ABaseEnemy* Enemy = Cast<ABaseEnemy>(AICon->GetPawn());
	if (!Enemy) return EBTNodeResult::Failed;

	Enemy->RequestAbilityByTag(AbilityTag);

	if (Enemy)
	{
		Enemy->EndCurrentBehavior();
	}

	return EBTNodeResult::Succeeded;
}
