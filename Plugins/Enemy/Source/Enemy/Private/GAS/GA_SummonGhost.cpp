// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_SummonGhost.h"

#include "BaseEnemy.h"
#include "SummonedAIController.h"
#include "Interfaces/BossAttack.h"

void UGA_SummonGhost::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                      const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	IBossAttack* BossAttacker = Cast<IBossAttack>(ActorInfo->AvatarActor.Get());
	if (!BossAttacker)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!BossAttacker->IsReadyToSummon())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	/* 구해온 위치에 Ghost 스폰 **/
	for (const FVector& Location : BossAttacker->GetSummonLocation())
	{
		ABaseEnemy* Ghost = GetWorld()->SpawnActor<ABaseEnemy>(SummonClass, Location, FRotator::ZeroRotator);
		if (Ghost)
		{
			// Ghost는 자기를 Summon한 Boss에 대해 WeakPtr 가지고 있음
			if (ASummonedAIController* SummonedAIController = Cast<ASummonedAIController>(Ghost->GetController()))
			{
				BossAttacker->AddSpawnedEnemy(Ghost);
				SummonedAIController->SetSummonOwner(Cast<UObject>(BossAttacker));
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}
