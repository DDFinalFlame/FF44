// Fill out your copyright notice in the Description page of Project Settings.


#include "BT/BTTask_Patrol.h"

#include "AIController.h"
#include "BaseEnemy.h"
#include "BehaviorTree/BlackboardComponent.h"

EBTNodeResult::Type UBTTask_Patrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	APawn* ControlledPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (!ControlledPawn)
	{
		return EBTNodeResult::Failed;
	}

	if (ABaseEnemy* Character = Cast<ABaseEnemy>(ControlledPawn))
	{
		//OwnerComp.GetBlackboardComponent()->SetValueAsVector(BlackboardLocation.SelectedKeyName, Character->GetPatrolPoint()->GetActorLocation());
		//Character->IncrementPatrolIndex();
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
