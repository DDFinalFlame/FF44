// Fill out your copyright notice in the Description page of Project Settings.


#include "BT/BTTask_TriggerAbility.h"

#include "AIController.h"
#include "BaseEnemy.h"

EBTNodeResult::Type UBTTask_TriggerAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	ABaseEnemy* Enemy = Cast<ABaseEnemy>(AICon->GetPawn());
	if (!Enemy) return EBTNodeResult::Failed;

	UAbilitySystemComponent* ASC = Enemy->FindComponentByClass<UAbilitySystemComponent>();
	if (!ASC) return EBTNodeResult::Failed;

	// Enemy 클래스 통해 Ability 발동 요청
	FGameplayAbilitySpecHandle Handle = Enemy->RequestAbilityByTag(AbilityTag);

	if (Handle == FGameplayAbilitySpecHandle())
	{
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::Succeeded;

}
