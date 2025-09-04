// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_PerformSummonAttack.h"

#include "Interfaces/BossAttack.h"

void UGA_PerformSummonAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// 기본 Attack 로직 ( Anim Montage 실행 -> Montage 실행과 맞춰 EndAbility 호출 )
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Summon 관련 함수 실행
	if (IBossAttack* Boss = Cast<IBossAttack>(ActorInfo->AvatarActor.Get()))
	{
		Boss->RequestSummonLocation();
	}

	//EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);

}
