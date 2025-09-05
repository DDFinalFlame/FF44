// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Interfaces/EnemyAIState.h"
#include "SummonedAIController.generated.h"

class IBossAttack;
class ABaseEnemy;
/**
 * 
 */
UCLASS()
class ENEMY_API ASummonedAIController : public AAIController, public IEnemyAIState
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	ACharacter* Target;

	UPROPERTY()
	ABaseEnemy* ControlledEnemy;

	UPROPERTY()
	TWeakObjectPtr<UObject> SummonOwner;

public:
	ASummonedAIController();

public:
	virtual  void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void Tick(float DeltaSeconds) override;
public:
	FORCEINLINE void SetSummonOwner(UObject* InSummonOwner ) { SummonOwner = InSummonOwner; };
public:
	// State Interface
	virtual EAIBehavior GetCurrentBehavior() override;
};
