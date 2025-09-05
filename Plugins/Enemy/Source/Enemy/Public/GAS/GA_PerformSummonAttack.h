// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GA_PerformAttack.h"
#include "GA_PerformSummonAttack.generated.h"

/**
 * 
 */
UCLASS()
class ENEMY_API UGA_PerformSummonAttack : public UGA_PerformAttack
{
	GENERATED_BODY()

protected:
virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
};
