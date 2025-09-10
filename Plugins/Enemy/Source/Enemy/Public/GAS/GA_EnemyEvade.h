// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_EnemyEvade.generated.h"

/**
 * 
 */
UCLASS()
class ENEMY_API UGA_EnemyEvade : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "GE")
	TSubclassOf<UGameplayEffect> EvadeEffectClass;
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// Task 종료와 바인딩해줄 함수
	UFUNCTION()
	void OnDamagedOnEvade(FGameplayEventData Payload);

	UFUNCTION()
	void OnEvadeEnd(FGameplayEventData Payload);
};
