// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_ReturnToBoss.h"

#include "AIController.h"
#include "BaseEnemy.h"
#include "BehaviorTree/BlackboardComponent.h"

void UGA_ReturnToBoss::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	/* ���� ��� valid üũ **/
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ABaseEnemy* Enemy = Cast<ABaseEnemy>(ActorInfo->AvatarActor.Get());
	if (!Enemy)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const ABaseEnemy* Boss = Cast<ABaseEnemy>(TriggerEventData->Instigator.Get());
	if (!Boss) { return; }

	FVector TargetLocation = Boss->GetActorLocation();

	// BB ��������
	if (AAIController* AIController = Cast<AAIController>(Enemy->GetController()))
	{
		if (UBlackboardComponent* BB = AIController->GetBlackboardComponent())
		{
			// State �ٲ��ְ�
			BB->SetValueAsEnum(BehaviorKeyName, static_cast<uint8>(EAIBehavior::Patrol));
			// ���ư� ��ġ BB�� ����
			BB->SetValueAsVector(TargetKeyName, TargetLocation);
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);

}
