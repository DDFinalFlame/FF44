// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_ReturnToBoss.h"

#include "AIController.h"
#include "BaseEnemy.h"
#include "BehaviorTree/BlackboardComponent.h"

void UGA_ReturnToBoss::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	/* 관련 요소 valid 체크 **/
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

	FVector Location = Boss->GetActorLocation();

	// BB 가져오기
	if (AAIController* AIController = Cast<AAIController>(Enemy->GetController()))
	{
		if (UBlackboardComponent* BB = AIController->GetBlackboardComponent())
		{
			// State 바꿔주고
			BB->SetValueAsVector(TargetLocationKeyName, Location);
			// 돌아갈 위치 BB에 저장
			BB->SetValueAsEnum(BehaviorKeyName, static_cast<uint8>(EAIBehavior::Patrol));
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);

}
