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
		/* AI Controller�� ���� Target �����ͼ� ��ġ �� **/
		UBlackboardComponent* BB = AIController->GetBlackboardComponent();
		AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject("Target"));
		FVector TargetLocation = TargetActor->GetActorLocation();

		FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
		return Direction;
	}

	return FVector(0.0f);
}
