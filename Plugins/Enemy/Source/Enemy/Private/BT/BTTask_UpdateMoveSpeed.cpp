// Fill out your copyright notice in the Description page of Project Settings.


#include "BT/BTTask_UpdateMoveSpeed.h"

#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"

EBTNodeResult::Type UBTTask_UpdateMoveSpeed::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	APawn* ControlledPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (!ControlledPawn)
	{
		return EBTNodeResult::Failed;
	}

	if (UCharacterMovementComponent* MovementComponent = ControlledPawn->GetComponentByClass<UCharacterMovementComponent>())
	{
		MovementComponent->MaxWalkSpeed = MaxWalkSpeed;
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
