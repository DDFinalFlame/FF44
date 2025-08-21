// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

class ABaseEnemy;
/**
 * 
 */
UCLASS()
class ENEMY_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
protected:
	/* AI가 주변 인식하게 하는 컴포넌트 **/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UAIPerceptionComponent* AIPerceptionComponent;

	UPROPERTY()
	ABaseEnemy* ControlledEnemy;

	FTimerHandle TimerHandle;
public:
	AEnemyAIController();

public:
	virtual  void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

public:
	void UpdateTarget() const;
	void SetTarget(AActor* NewTarget) const;
	void SetNoiseTarget(AActor* NewTarget) const;
};
