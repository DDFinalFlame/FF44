// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_Recall.h"

#include "BaseEnemy.h"
#include "Interfaces/BossAttack.h"

void UGA_Recall::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	IBossAttack* BossAttacker = Cast<IBossAttack>(ActorInfo->AvatarActor.Get());
	if (!BossAttacker)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	for (auto Ghost : BossAttacker->GetGhostList())
	{
		// Owner가 Ability System Component를 가진 경우
		UAbilitySystemComponent* ASC = Ghost->FindComponentByClass<UAbilitySystemComponent>();

		if (!ASC) return;

		// FGameplayEventData 생성
		FGameplayEventData EventData;
		EventData.Instigator = Ghost.Get();
		EventData.Target = nullptr;
		EventData.EventTag = EventTag;

		// Event 전송
		ASC->HandleGameplayEvent(EventTag, &EventData);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}
