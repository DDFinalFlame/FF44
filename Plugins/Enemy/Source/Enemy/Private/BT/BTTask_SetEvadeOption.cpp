// Fill out your copyright notice in the Description page of Project Settings.


#include "BT/BTTask_SetEvadeOption.h"

#include "AIController.h"
#include "Interfaces/EnemyEvade.h"

EBTNodeResult::Type UBTTask_SetEvadeOption::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Valid 체크 
	APawn* ControlledPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (!ControlledPawn) { return EBTNodeResult::Failed; }

	IEnemyEvade* Enemy = Cast<IEnemyEvade>(ControlledPawn);
	if (!Enemy) { return EBTNodeResult::Failed; }


	// Owner의 충돌 비활성화
	Enemy->ToggleCollision(bStartEvade);

	// Owner의 FX 활성화
	//Enemy->ToggleFX(bStartEvade);

	return EBTNodeResult::Succeeded;
}
