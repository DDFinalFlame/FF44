#include "RangedEnemy/RangedEnemy.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

ARangedEnemy::ARangedEnemy()
{
	// Bow Mesh ���̱�
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>("Weapon Mesh");
}

void ARangedEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (USkeletalMeshComponent* SkeletalMesh = GetMesh())
	{
		if (SocketName.IsValid() && WeaponMesh)
		{
			WeaponMesh->AttachToComponent(SkeletalMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
		}
	}
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
		AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject("F_Target"));
		FVector TargetLocation = TargetActor->GetActorLocation();

		FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
		return Direction;
	}

	return FVector(0.0f);
}
