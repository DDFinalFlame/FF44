#include "RangedEnemy/RangedEnemy.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

ARangedEnemy::ARangedEnemy()
{

}

FVector ARangedEnemy::GetMuzzleLocation()
{
	FVector MuzzleLocation = GetActorLocation() + FVector(0, 0, 50);
	return MuzzleLocation;
}

FVector ARangedEnemy::GetMuzzleDirection()
{
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		/* AI Controller를 통해 Target 가져와서 위치 비교 **/
		UBlackboardComponent* BB = AIController->GetBlackboardComponent();
		AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject("Target"));
		FVector TargetLocation = TargetActor->GetActorLocation();

		FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
		return Direction;
	}

	return FVector(0.0f);
}
