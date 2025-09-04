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

	/* ���ؿ� ��ġ�� Ghost ���� **/
	for (const FVector& Location : BossAttacker->GetSummonLocation())
	{
		ABaseEnemy* Ghost = GetWorld()->SpawnActor<ABaseEnemy>(SummonClass, Location, FRotator::ZeroRotator);
		if (Ghost)
		{
			// Ghost�� �ڱ⸦ Summon�� Boss�� ���� WeakPtr ������ ����
			if (ASummonedAIController* SummonedAIController = Cast<ASummonedAIController>(Ghost->GetController()))
			{
				BossAttacker->AddSpawnedEnemy(Ghost);
				SummonedAIController->SetSummonOwner(Cast<UObject>(BossAttacker));
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}
