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

    if (!TargetActor) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Target 방향 계산
    FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(
        Owner->GetActorLocation(),
        TargetActor->GetActorLocation()
    );
    TargetRotation.Pitch = 0.f;
    TargetRotation.Roll = 0.f;

    // 부드러운 회전
    FRotator R = FMath::RInterpConstantTo(Owner->GetActorRotation(),
        TargetRotation,
        DeltaTime,
        RotationSpeed
    );

    Owner->SetActorRotation(R);

}

