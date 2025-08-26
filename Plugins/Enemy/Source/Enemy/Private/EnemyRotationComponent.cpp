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

	if (!TargetActor)
	{
		return;
	}

	if (!bShouldRotate)
	{
		return;
	}

	const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetOwner()->GetActorLocation(), TargetActor->GetActorLocation());
	GetOwner()->SetActorRotation(FRotator(0.0f, LookAtRotation.Yaw, 0.0f));

}

