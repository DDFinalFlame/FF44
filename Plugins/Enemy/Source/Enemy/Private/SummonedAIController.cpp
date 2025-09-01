// Fill out your copyright notice in the Description page of Project Settings.


#include "SummonedAIController.h"

#include "BaseEnemy.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Interfaces/BossAttack.h"
#include "Kismet/GameplayStatics.h"


ASummonedAIController::ASummonedAIController()
{
}

void ASummonedAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	/* Enemy ���� **/
	ControlledEnemy = Cast<ABaseEnemy>(InPawn);
	/* Target ( Player ) ���� **/
	Target = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (ControlledEnemy->BehaviorTree)
	{
		RunBehaviorTree(ControlledEnemy->BehaviorTree);
		Blackboard->SetValueAsObject(FName("F_Target"), Target);
	}

}

void ASummonedAIController::OnUnPossess()
{
	// BT, BB
	if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BrainComponent))
	{
		BTComp->StopTree(EBTStopMode::Safe);
	}

	// Owner�� �˸���
	Cast<IBossAttack>(SummonOwner)->DeleteSpawnedEnemy(ControlledEnemy);

	ControlledEnemy = nullptr;
	Target = nullptr;

	Super::OnUnPossess();
}

void ASummonedAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (UEnemyRotationComponent* RotationComponent = ControlledEnemy->GetComponentByClass<UEnemyRotationComponent>())
	{
		RotationComponent->SetTargetLocation(Target->GetActorLocation());
	}

}