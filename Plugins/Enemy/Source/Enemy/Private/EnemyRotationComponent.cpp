#include "EnemyRotationComponent.h"

#include "Kismet/KismetMathLibrary.h"

UEnemyRotationComponent::UEnemyRotationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UEnemyRotationComponent::BeginPlay()
{
	Super::BeginPlay();

	
}


void UEnemyRotationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bShouldRotate) { return; }
    if (!IsVectorValidForUse(TargetLocation)) { return; }
	//if (!TargetActor) { return; }

    AActor* Owner = GetOwner();
    if (!Owner) return;

    //// Target ���� ��� ( TargetActor�� ����� ��� )
    //FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(
    //    Owner->GetActorLocation(),
    //    TargetActor->GetActorLocation()
    //);
    //TargetRotation.Pitch = 0.f;
    //TargetRotation.Roll = 0.f;

    // Target ���� ��� ( TargetLocation�� ����� ��� )
	FRotator TargetRotationWithV = UKismetMathLibrary::FindLookAtRotation(
        Owner->GetActorLocation(),
        TargetLocation
    );
    TargetRotationWithV.Pitch = 0.f;
    TargetRotationWithV.Roll = 0.f;

	// �ε巯�� ȸ��
    FRotator R = FMath::RInterpConstantTo(Owner->GetActorRotation(),
        TargetRotationWithV,
        DeltaTime,
        RotationSpeed
    );

    Owner->SetActorRotation(R);

}

bool UEnemyRotationComponent::IsVectorValidForUse(const FVector& Vec, float Tolerance)
{
    // 1. NaN / Inf üũ
    if (Vec.ContainsNaN())
    {
        return false;
    }

    if (!FMath::IsFinite(Vec.X) || !FMath::IsFinite(Vec.Y) || !FMath::IsFinite(Vec.Z))
    {
        return false;
    }

    // 2. �ʹ� ���� ��(���� Zero Vector) üũ
    if (Vec.IsNearlyZero(Tolerance))
    {
        return false;
    }

    return true;
}

