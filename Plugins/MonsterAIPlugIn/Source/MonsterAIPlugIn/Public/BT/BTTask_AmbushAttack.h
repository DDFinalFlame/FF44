// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_AmbushAttack.generated.h"

UCLASS(Blueprintable)
class MONSTERAIPLUGIN_API UBTTask_AmbushAttack : public UBTTaskNode
{
	GENERATED_BODY()
	
    public:
    UBTTask_AmbushAttack()
    {
        NodeName = TEXT("Ambush Attack (Attack + SwitchToCombatReady)");
    }

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
