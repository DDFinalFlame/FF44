// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"

#include "BaseEnemy.h"
#include "EnemyRotationComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Sight.h"

AEnemyAIController::AEnemyAIController()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("AI Perception");
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	/* Enemy 저장 **/
	ControlledEnemy = Cast<ABaseEnemy>(InPawn);

	if (ControlledEnemy->BehaviorTree)
	{
		RunBehaviorTree(ControlledEnemy->BehaviorTree);
	}

	/* UpdateTarget 타이머 등록 **/
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::UpdateTarget, 0.1f, true);
}

void AEnemyAIController::OnUnPossess()
{
	// BT, BB
	if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BrainComponent)) 
	{
		BTComp->StopTree(EBTStopMode::Safe);
	}
	/* 이렇게 초기화해도 되려나 .. ?**/
	Blackboard->InitializeBlackboard(*Blackboard->GetBlackboardAsset());

	ControlledEnemy = nullptr;
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);

	Super::OnUnPossess();
}

void AEnemyAIController::UpdateTarget() const
{
	TArray<AActor*> SightActors;
	TArray<AActor*> HearingActors;
	PerceptionComponent->GetKnownPerceivedActors(UAISense_Sight::StaticClass(), SightActors);
	PerceptionComponent->GetKnownPerceivedActors(UAISense_Hearing::StaticClass(), HearingActors);

	ACharacter* PlayerCharacter = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	/* Sight 확인 **/
	if (SightActors.Contains(PlayerCharacter))
	{
		SetTarget(PlayerCharacter);
		return;

		//if (!PlayerCharacter->isDeath())
		//{d
		//	SetTarget(PlayerCharacter);
		//	ControlledEnemy->ToggleHPBarVisibility(true);
		//}
		//else
		//{
		//	SetTarget(nullptr);
		//	ControlledEnemy->ToggleHPBarVisibility(false);
		//}
	}
	else
	{
		SetTarget(nullptr);
		//ControlledEnemy->ToggleHPBarVisibility(false);
	}

	/* hearing 확인 **/
	if (HearingActors.Contains(PlayerCharacter))
	{
		SetNoiseTarget(PlayerCharacter);
	}
	else
	{
		SetNoiseTarget(nullptr);
	}
}

void AEnemyAIController::SetTarget(AActor* NewTarget) const
{
	if (IsValid(Blackboard))
	{
		Blackboard->SetValueAsObject(FName("F_Target"), NewTarget);
	}

	if (UEnemyRotationComponent* RotationComponent = ControlledEnemy->GetComponentByClass<UEnemyRotationComponent>())
	{
		RotationComponent->SetTargetActor(NewTarget);
	}
}

void AEnemyAIController::SetNoiseTarget(AActor* NewTarget) const
{
	if (IsValid(Blackboard))
	{
		Blackboard->SetValueAsObject(FName("F_NoiseTarget"), NewTarget);
	}

	if (UEnemyRotationComponent* RotationComponent = ControlledEnemy->GetComponentByClass<UEnemyRotationComponent>())
	{
		RotationComponent->SetTargetActor(NewTarget);
	}
}
