#include "BTTask_RotateEnemy.h"

#include "AIController.h"
#include "BaseEnemy.h"
#include "EnemyRotationComponent.h"

EBTNodeResult::Type UBTTask_RotateEnemy::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	APawn* ControlledPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (!ControlledPawn)
	{
		return EBTNodeResult::Failed;
	}

	if (ABaseEnemy* Character = Cast<ABaseEnemy>(ControlledPawn))
	{
		if (UEnemyRotationComponent* RotationComponent = Character->GetComponentByClass<UEnemyRotationComponent>())
		{
			RotationComponent->ToggleShouldRotate(CanRotate);
		}
	}

	return EBTNodeResult::Succeeded;
}
