// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_PerformSummonAttack.h"

#include "Interfaces/BossAttack.h"

void UGA_PerformSummonAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// �⺻ Attack ���� ( Anim Montage ���� -> Montage ����� ���� EndAbility ȣ�� )
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Summon ���� �Լ� ����
	if (IBossAttack* Boss = Cast<IBossAttack>(ActorInfo->AvatarActor.Get()))
	{
		Boss->RequestSummonLocation();
	}

	//EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);

}
