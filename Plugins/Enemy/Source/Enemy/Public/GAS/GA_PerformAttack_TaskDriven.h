// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_PerformAttack_TaskDriven.generated.h"

/**
 * 
 */
UCLASS()
class ENEMY_API UGA_PerformAttack_TaskDriven : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_PerformAttack_TaskDriven();

	// attack 유형별 설정
protected:
	/* 몽타주 블랜딩 타임 설정**/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
	float BlendingTime = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttackTag")
	FGameplayTag AbilityTag;


protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UFUNCTION()
	void OnEndTask(FGameplayEventData Payload);
};
