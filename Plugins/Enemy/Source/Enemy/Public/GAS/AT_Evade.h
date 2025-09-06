// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AT_Evade.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEnemyEvadeEvent);

struct FEnvQueryResult;
class UEnvQuery;
/**
 * 
 */
UCLASS()
class ENEMY_API UAT_Evade : public UAbilityTask
{
	GENERATED_BODY()

	UPROPERTY(BlueprintAssignable)
	FEnemyEvadeEvent OnReached;

	UPROPERTY(BlueprintAssignable)
	FEnemyEvadeEvent OnFailed;

protected:
	// Evade End Pos 찾을 EQS
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EQS")
	UEnvQuery* EndPosQueryTemplate;

	// EQS 결과
	TArray<FVector> CachedLocations;


public:
	UAT_Evade(const FObjectInitializer& ObjectInitializer);
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", DefaultToSelf = "OwningAbility"))
	static UAT_Evade* AbilityTick(UGameplayAbility* OwningAbility);

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;

public:
	// EQS Finish 바인딩할 델리게이트 함수
	void OnQueryFinished(TSharedPtr<FEnvQueryResult> Result);
	
};
