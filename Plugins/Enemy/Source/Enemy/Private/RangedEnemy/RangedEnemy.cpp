#include "RangedEnemy/RangedEnemy.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

ARangedEnemy::ARangedEnemy()
{
	// Bow Mesh 붙이기
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
		/* AI Controller를 통해 Target 가져와서 위치 비교 **/
		UBlackboardComponent* BB = AIController->GetBlackboardComponent();
		AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject("F_Target"));
		FVector TargetLocation = TargetActor->GetActorLocation();

		FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
		return Direction;
	}

	return FVector(0.0f);
}

void ARangedEnemy::GetAllMetarials(TArray<UMaterialInstanceDynamic*>& OutArray)
{
	// 위치를 ....
	DropBow();

	// Main Mesh
	Super::GetAllMetarials(OutArray);

	int32 num = GetMesh()->GetNumChildrenComponents();
	for (int32 numMesh = 0; numMesh < num; ++numMesh)
	{
		if (UMeshComponent* ChildMesh = Cast<UMeshComponent>(GetMesh()->GetChildComponent(numMesh)))
		{
			int32 MaterialCount = ChildMesh->GetNumMaterials();

			for (int32 numMat = 0; numMat < MaterialCount; ++numMat)
			{
				UMaterialInstanceDynamic* DynMat = ChildMesh->CreateAndSetMaterialInstanceDynamic(numMat);
				if (DynMat)
				{
					OutArray.Add(DynMat);
				}
			}
		}
		
	}
}

void ARangedEnemy::DropBow()
{
	if (WeaponMesh)
	{
		WeaponMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);


		WeaponMesh->SetSimulatePhysics(true);     // 물리 시뮬레이션 활성화
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // 충돌 활성화
		WeaponMesh->SetCollisionObjectType(ECC_PhysicsBody); // 충돌 타입 지정

		WeaponMesh->AddImpulse(Power, NAME_None, true);
	}
}
