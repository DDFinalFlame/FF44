// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseEnemy.h"
#include "SummonedEnemy.generated.h"

/**
 * 
 */
UCLASS()
class ENEMY_API ASummonedEnemy : public ABaseEnemy
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS | Effect")
	TSubclassOf<UGameplayEffect> BuffEffectCDO;

public:
	ASummonedEnemy();

protected:
	virtual void BeginPlay() override;

protected:
	UFUNCTION()
	void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnDestroySummonedEnemy();

public:
	virtual void OnDeath() override;

};
