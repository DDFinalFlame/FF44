// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_AttackPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "MonsterCharacter.h"
#include "Animation/AnimInstance.h"

UBTTask_AttackPlayer::UBTTask_AttackPlayer()
{
	NodeName = TEXT("Attack Player");
	bNotifyTick = true; // TickTask 사용 가능하게 설정
	MontageDuration = 0.0f;
	ElapsedTime = 0.0f;
}

EBTNodeResult::Type UBTTask_AttackPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	AMonsterCharacter* EnemyCharacter = Cast<AMonsterCharacter>(AIController->GetPawn());
	if (!EnemyCharacter) return EBTNodeResult::Failed;

	UAnimInstance* AnimInstance = EnemyCharacter->GetMesh()->GetAnimInstance();
	if (!AnimInstance) return EBTNodeResult::Failed;

	if (EnemyCharacter->AttackMontage)
	{
		float PlayedLen = AnimInstance->Montage_Play(EnemyCharacter->AttackMontage, 1.0f);
		if (PlayedLen > 0.f)
		{
			MontageDuration = EnemyCharacter->AttackMontage->GetPlayLength();
			ElapsedTime = 0.0f;
			return EBTNodeResult::InProgress;
		}
	}

	return EBTNodeResult::Failed;
}

void UBTTask_AttackPlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	ElapsedTime += DeltaSeconds;
	if (ElapsedTime >= MontageDuration)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

