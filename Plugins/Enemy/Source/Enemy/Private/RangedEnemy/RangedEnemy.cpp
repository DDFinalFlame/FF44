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

void ARangedEnemy::GetAllMetarials(TArray<UMaterialInstanceDynamic*>& OutArray)
{
	// ��ġ�� ....
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


		WeaponMesh->SetSimulatePhysics(true);     // ���� �ùķ��̼� Ȱ��ȭ
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // �浹 Ȱ��ȭ
		WeaponMesh->SetCollisionObjectType(ECC_PhysicsBody); // �浹 Ÿ�� ����

		WeaponMesh->AddImpulse(Power, NAME_None, true);
	}
}
