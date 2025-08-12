// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MonsterAIController.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERAIPLUGIN_API AMonsterAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AMonsterAIController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
    virtual void OnPossess(APawn* InPawn) override;
	
	void UpdateBlackboardKeys();

protected:
    //Behavior Tree Asset 지정용 (에디터에서 설정 가능)
    UPROPERTY(EditAnywhere, Category = "AI")
    class UBehaviorTree* BehaviorTreeAsset;

    //Blackboard Asset 지정용 (에디터에서 설정 가능)
    UPROPERTY(EditAnywhere, Category = "AI")
    class UBlackboardData* BBAsset;

    //BlackboardComponent (내부 사용용)
    UPROPERTY()
    class UBlackboardComponent* BlackboardComponent;
};
