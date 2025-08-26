// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAnimInstance.h"

#include "BaseEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Runtime/AnimGraphRuntime/Public/KismetAnimationLibrary.h"

UEnemyAnimInstance::UEnemyAnimInstance()
{
}

void UEnemyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Enemy = Cast<ABaseEnemy>(GetOwningActor());

	if (Enemy)
	{
		MovementComponent = Enemy->GetCharacterMovement();
		CurrentBehavior = Enemy->GetCurrentBehavior();
	}

}

void UEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (Enemy == nullptr)
	{
		return;
	}

	if (MovementComponent == nullptr)
	{
		return;
	}

	/* State 관련 **/
	CurrentBehavior = Enemy->GetCurrentBehavior();

	/* Movement 관련 **/
	Velocity = MovementComponent->Velocity;
	GroundSpeed = Velocity.Size2D();
	bShouldMove = GroundSpeed > 3.0f && MovementComponent->GetCurrentAcceleration() != FVector::ZeroVector;
	bIsFalling = MovementComponent->IsFalling();
	Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, Enemy->GetActorRotation());
}