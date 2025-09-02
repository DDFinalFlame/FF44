// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_ReturnToBoss.generated.h"

/**
 * 
 */
UCLASS()
class ENEMY_API UGA_ReturnToBoss : public UGameplayAbility
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TargetKeyName = "F_Target";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BehaviorKeyName = "Behavior";


protected:
	virtual auto ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) -> void override;

};
