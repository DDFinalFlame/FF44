// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_PerformAttack.generated.h"

/**
 * 
 */
UCLASS()
class ENEMY_API UGA_PerformAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_PerformAttack();

// attack 유형별 설정
protected:
	/* 몽타주 블랜딩 타임 설정**/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
	float BlendingTime = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttackTag")
	FGameplayTag AbilityTag;

	//// Sound ( 더 정확한 타이밍 제어를 위해 AM에서 ) 
	//UPROPERTY(EditAnywhere, Category = "Audio")
	//USoundBase* AttackSound;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	//UFUNCTION()
	//void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	//UFUNCTION()
	//void OnMontageBlendedIn(UAnimMontage* Montage);
	//UFUNCTION()
	//void OnMontageBlendedOut(UAnimMontage* Montage, bool bSth);

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnMontageBlendOut();
};
