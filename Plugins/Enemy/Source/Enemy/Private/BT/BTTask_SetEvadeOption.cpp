// Fill out your copyright notice in the Description page of Project Settings.


#include "BT/BTTask_SetEvadeOption.h"

#include "AIController.h"
#include "Interfaces/EnemyEvade.h"

EBTNodeResult::Type UBTTask_SetEvadeOption::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Valid üũ 
	APawn* ControlledPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (!ControlledPawn) { return EBTNodeResult::Failed; }

	IEnemyEvade* Enemy = Cast<IEnemyEvade>(ControlledPawn);
	if (!Enemy) { return EBTNodeResult::Failed; }


	// Owner�� �浹 ��Ȱ��ȭ
	Enemy->ToggleCollision(bStartEvade);

	// Owner�� FX Ȱ��ȭ
	//Enemy->ToggleFX(bStartEvade);

	return EBTNodeResult::Succeeded;
}
