#include "BT/BTTask_PatrolPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "GameFramework/Pawn.h"

UBTTask_PatrolPlayer::UBTTask_PatrolPlayer()
{
	NodeName = TEXT("Find Random Patrol Location");

	// Blackboard에서 Vector만 허용
	PatrolLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_PatrolPlayer, PatrolLocationKey));
}

EBTNodeResult::Type UBTTask_PatrolPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	APawn* AIPawn = AIController->GetPawn();
	if (AIPawn == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	FNavLocation RandomLocation;
	bool bFound = NavSys->GetRandomReachablePointInRadius(AIPawn->GetActorLocation(), SearchRadius, RandomLocation);

	if (bFound)
	{
		// Blackboard에 위치 저장
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(PatrolLocationKey.SelectedKeyName, RandomLocation.Location);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}