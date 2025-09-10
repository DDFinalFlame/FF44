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

	/* Enemy 저장 **/
	ControlledEnemy = Cast<ABaseEnemy>(InPawn);
	/* Target ( Player ) 저장 **/
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

	// Owner에 알리기
	if (IBossAttack* Boss = Cast<IBossAttack>(SummonOwner))
	{
		Boss->DeleteSpawnedEnemy(ControlledEnemy);
	}
	

	ControlledEnemy = nullptr;
	Target = nullptr;

	SetActorTickEnabled(false);

	Super::OnUnPossess();
}

void ASummonedAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//if (!ControlledEnemy) { return; }
	IBossAttack* Boss = Cast<IBossAttack>(SummonOwner);
	if (!Boss) { return;  }

	if (UEnemyRotationComponent* RotationComponent = ControlledEnemy->GetComponentByClass<UEnemyRotationComponent>())
	{
		EAIBehavior CurrentBehavior = GetCurrentBehavior();
		switch (CurrentBehavior)
		{
		case EAIBehavior::Patrol:
			RotationComponent->SetTargetLocation(Cast<IBossAttack>(SummonOwner)->GetBossLocation());
			break;
		default:
			RotationComponent->SetTargetLocation(Target->GetActorLocation());
			break;
		}
	}

}

EAIBehavior ASummonedAIController::GetCurrentBehavior()
{
	return static_cast<EAIBehavior>(Blackboard->GetValueAsEnum("Behavior"));
}
