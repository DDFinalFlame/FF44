// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GA_PerformAttack.h"
#include "GA_PerformRangedAttack.generated.h"

class AProjectileBase;

/**
 * 
 */
UCLASS()
class ENEMY_API UGA_PerformRangedAttack : public UGA_PerformAttack
{
	GENERATED_BODY()

protected:
	/* 발사체 스폰 위치 **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	FVector WeaponLocation;

	/* 스폰할 발사체 클래스 **/ 
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<AProjectileBase> ProjectileClass;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
