// Fill out your copyright notice in the Description page of Project Settings.


#include "BT/BTService_SelectBossBehavior.h"

#include "AIController.h"
#include "BaseEnemy.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_SelectBossBehavior::UBTService_SelectBossBehavior()
{
	INIT_SERVICE_NODE_NOTIFY_FLAGS();
}

void UBTService_SelectBossBehavior::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
}

void UBTService_SelectBossBehavior::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	if (APawn* ControlledPawn = OwnerComp.GetAIOwner()->GetPawn())
	{
		if (ABaseEnemy* ControlledEnemy = Cast<ABaseEnemy>(ControlledPawn))
		{
			UpdateBehavior(OwnerComp.GetBlackboardComponent(), ControlledEnemy);

		}
	}
}

void UBTService_SelectBossBehavior::SetBehaviorKey(UBlackboardComponent* BlackboardComponent, EAIBehavior Behavior) const
{
	BlackboardComponent->SetValueAsEnum(BehaviorKey.SelectedKeyName, static_cast<uint8>(Behavior));
}

void UBTService_SelectBossBehavior::UpdateBehavior(UBlackboardComponent* BlackboardComponent, ABaseEnemy* ControlledEnemy)
{
	check(BlackboardComponent);
	check(ControlledEnemy);

	if (ControlledEnemy->GetCurrentBehavior() == EAIBehavior::Die) { return; }

	AActor* TargetActor = Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetKey.SelectedKeyName));

	if (!IsValid(TargetActor)) { return; }

	const float Distance = TargetActor->GetDistanceTo(ControlledEnemy);
	const int32 Phase = BlackboardComponent->GetValueAsInt(PhaseKey.SelectedKeyName);
	const bool bOpeningPatternDone = BlackboardComponent->GetValueAsBool(bOpeningPatternDoneKey.SelectedKeyName);

	TArray<EAIBehavior> Candidates;

	if (Phase == 1)
	{
		Candidates = { EAIBehavior::MeleeAttack, EAIBehavior::Evade };
	}
	else if (Phase == 2)
	{
		if (!bOpeningPatternDone)
		{
			BlackboardComponent->SetValueAsEnum(BehaviorKey.SelectedKeyName, (uint8)EAIBehavior::Summon);
			BlackboardComponent->SetValueAsBool(bOpeningPatternDoneKey.SelectedKeyName, true);
			return;
		}
		Candidates = { EAIBehavior::MeleeAttack, EAIBehavior::Evade, EAIBehavior::Summon };
	}
	else if (Phase == 3)
	{
		if (!bOpeningPatternDone)
		{
			BlackboardComponent->SetValueAsEnum(BehaviorKey.SelectedKeyName, (uint8)EAIBehavior::Grab);
			BlackboardComponent->SetValueAsBool(bOpeningPatternDoneKey.SelectedKeyName, true);
			return;
		}
		Candidates = { EAIBehavior::MeleeAttack, EAIBehavior::Evade, EAIBehavior::Summon, EAIBehavior::Grab };
	}

	if (Candidates.Num() > 0)
	{
		const int32 Index = FMath::RandRange(0, Candidates.Num() - 1);
		BlackboardComponent->SetValueAsEnum(BehaviorKey.SelectedKeyName, (uint8)Candidates[Index]);
	}

}


