// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_SelectBehavior.h"

#include "AIController.h"
#include "BaseEnemy.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DSP/Osc.h"

UBTService_SelectBehavior::UBTService_SelectBehavior()
{
	INIT_SERVICE_NODE_NOTIFY_FLAGS();
}

void UBTService_SelectBehavior::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	APawn* ControlledPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	ControlledEnemy = Cast<ABaseEnemy>(ControlledPawn);
}

void UBTService_SelectBehavior::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UpdateBehavior(OwnerComp.GetBlackboardComponent());
}

void UBTService_SelectBehavior::SetBehaviorKey(UBlackboardComponent* BlackboardComponent, EAIBehavior Behavior) const
{
	BlackboardComponent->SetValueAsEnum(BehaviorKey.SelectedKeyName, static_cast<uint8>(Behavior));
}

void UBTService_SelectBehavior::UpdateBehavior(UBlackboardComponent* BlackboardComponent)
{
	check(BlackboardComponent);
	check(ControlledEnemy);

	AActor* TargetActor = Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetKey.SelectedKeyName));
	AActor* NoiseTargetActor = Cast<AActor>(BlackboardComponent->GetValueAsObject(NoiseTargetKey.SelectedKeyName));


	if (IsValid(TargetActor))
	{
		const float Distance = TargetActor->GetDistanceTo(ControlledEnemy);

		/* 공격 범위 안쪽이면 **/
		if (Distance <= AttackRangeDistance)
		{
			SetBehaviorKey(BlackboardComponent, EAIBehavior::MeleeAttack);
		}
		/* 아직 멀다 **/
		else
		{
			SetBehaviorKey(BlackboardComponent, EAIBehavior::Approach);
		}
	}
	else if (IsValid(NoiseTargetActor))
	{
		SetBehaviorKey(BlackboardComponent, EAIBehavior::Investigate);
	}
	else
	{
		/* Patrol Point 가 있다면 **/
		if (ControlledEnemy->GetPatrolPoint() != nullptr)
		{
			SetBehaviorKey(BlackboardComponent, EAIBehavior::Patrol);
		}
		else
		{
			SetBehaviorKey(BlackboardComponent, EAIBehavior::Idle);
		}
	}
}
