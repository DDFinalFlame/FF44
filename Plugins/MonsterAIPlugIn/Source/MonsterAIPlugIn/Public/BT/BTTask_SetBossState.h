// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Boss/BossCharacter.h"
#include "BTTask_SetBossState.generated.h"


UCLASS(Blueprintable)
class MONSTERAIPLUGIN_API UBTTask_SetBossState : public UBTTaskNode
{
    GENERATED_BODY()
public:
    UBTTask_SetBossState();

    UPROPERTY(EditAnywhere, Category = "BossState")
    FBlackboardKeySelector BossStateKey;

    UPROPERTY(EditAnywhere, Category = "BossState", meta = (GetOptions = "GetBossStateOptions"))
    EBossState_BB TargetBossState = EBossState_BB::Idle;

    UPROPERTY(EditAnywhere, Category = "BossState")
    bool bAlsoSetCharacterState = true;

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;

private:
    UFUNCTION()
    TArray<FString> GetBossStateOptions() const;
};
