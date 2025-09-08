// Fill out your copyright notice in the Description page of Project Settings.


#include "BT/BTTask_SetBossState.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Boss/BossCharacter.h"    

UBTTask_SetBossState::UBTTask_SetBossState()
{
    NodeName = TEXT("Set Boss State (Enum)");
    bCreateNodeInstance = true; 
}

EBTNodeResult::Type UBTTask_SetBossState::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!BB || !AIC) return EBTNodeResult::Failed;

    if (!BossStateKey.SelectedKeyType || !BossStateKey.SelectedKeyName.IsNone())
    {
        BB->SetValueAsEnum(BossStateKey.SelectedKeyName, static_cast<uint8>(TargetBossState));
    }
    else
    {
        return EBTNodeResult::Failed;
    }


    return EBTNodeResult::Succeeded;
}

FString UBTTask_SetBossState::GetStaticDescription() const
{
    return FString::Printf(TEXT("Set BB[%s] = %d%s"),
        *BossStateKey.SelectedKeyName.ToString(),
        TargetBossState,
        bAlsoSetCharacterState ? TEXT(" (+Apply to Boss Character)") : TEXT(""));
}


TArray<FString> UBTTask_SetBossState::GetBossStateOptions() const
{
    TArray<FString> Out;
    return Out;
}
