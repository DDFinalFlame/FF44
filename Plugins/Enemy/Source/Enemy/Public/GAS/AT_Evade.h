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
	// Evade End Pos ã�� EQS
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EQS")
	UEnvQuery* EndPosQueryTemplate;

	// EQS ���
	TArray<FVector> CachedLocations;


public:
	UAT_Evade(const FObjectInitializer& ObjectInitializer);
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", DefaultToSelf = "OwningAbility"))
	static UAT_Evade* AbilityTick(UGameplayAbility* OwningAbility);

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;

public:
	// EQS Finish ���ε��� ��������Ʈ �Լ�
	void OnQueryFinished(TSharedPtr<FEnvQueryResult> Result);
	
};
