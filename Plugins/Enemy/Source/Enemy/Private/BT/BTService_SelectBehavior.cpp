// Fill out your copyright notice in the Description page of Project Settings.


#include "BT/BTService_SelectBehavior.h"

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
}

void UBTService_SelectBehavior::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	if (APawn* ControlledPawn = OwnerComp.GetAIOwner()->GetPawn())
	{
		if (ABaseEnemy* ControlledEnemy = Cast<ABaseEnemy>(ControlledPawn))
		{
			UpdateBehavior(OwnerComp.GetBlackboardComponent(), ControlledEnemy);

		}
		return;
	}
}

void UBTService_SelectBehavior::SetBehaviorKey(UBlackboardComponent* BlackboardComponent, EAIBehavior Behavior) const
{
	BlackboardComponent->SetValueAsEnum(BehaviorKey.SelectedKeyName, static_cast<uint8>(Behavior));
}

void UBTService_SelectBehavior::UpdateBehavior(UBlackboardComponent* BlackboardComponent, ABaseEnemy* ControlledEnemy)
{
	check(BlackboardComponent);
	check(ControlledEnemy);

	if (ControlledEnemy->GetCurrentBehavior() == EAIBehavior::Die) { return; }

	AActor* TargetActor = Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetKey.SelectedKeyName));
	AActor* NoiseTargetActor = Cast<AActor>(BlackboardComponent->GetValueAsObject(NoiseTargetKey.SelectedKeyName));

	if (IsValid(TargetActor))
	{
		const float Distance = TargetActor->GetDistanceTo(ControlledEnemy);

		/* 공격 범위 안쪽이면 **/
		if (Distance <= MeleeAttackRangeDistance)
		{
			if (ControlledEnemy->ChangeState(EAIBehavior::MeleeAttack))
			{
				SetBehaviorKey(BlackboardComponent, EAIBehavior::MeleeAttack);
			}
		}
		else if (Distance <= AttackRangeDistance)
		{
			if (ControlledEnemy->ChangeState(EAIBehavior::RangeAttack))
			{
				SetBehaviorKey(BlackboardComponent, EAIBehavior::RangeAttack);
			}
		}
		/* 아직 멀다 **/
		else
		{
			if (ControlledEnemy->ChangeState(EAIBehavior::Approach))
			{
				SetBehaviorKey(BlackboardComponent, EAIBehavior::Approach);

			}
		}
	}
	else if (IsValid(NoiseTargetActor))
	{
		if (ControlledEnemy->ChangeState(EAIBehavior::Investigate))
		{
			SetBehaviorKey(BlackboardComponent, EAIBehavior::Investigate);
		}
	}
	else
	{
		/* Patrol Point 가 있다면 **/
		if (ControlledEnemy->GetPatrolPoint() != nullptr)
		{
			if (ControlledEnemy->ChangeState(EAIBehavior::Patrol))
			{
				SetBehaviorKey(BlackboardComponent, EAIBehavior::Patrol);
			}
		}
		else
		{
			if (ControlledEnemy->ChangeState(EAIBehavior::Patrol))
			{
				SetBehaviorKey(BlackboardComponent, EAIBehavior::Idle);
			}
		}
	}
}