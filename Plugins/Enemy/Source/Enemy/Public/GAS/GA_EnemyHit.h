// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_EnemyHit.generated.h"

/**
 * 
 */
UCLASS()
class ENEMY_API UGA_EnemyHit : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_EnemyHit();
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EventTag")
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	TSubclassOf<UGameplayEffect> HitEffect;

	FVector AttackerLocation;

protected:
	virtual auto ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) -> void override;

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
