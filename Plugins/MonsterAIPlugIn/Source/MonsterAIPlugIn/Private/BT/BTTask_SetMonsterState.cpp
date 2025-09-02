// Fill out your copyright notice in the Description page of Project Settings.


#include "BT/BTTask_SetMonsterState.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monster/MonsterCharacter.h"  // SetMonsterState ����� ��

UBTTask_SetMonsterState::UBTTask_SetMonsterState()
{
    NodeName = TEXT("Set Monster State (Int)");
}

EBTNodeResult::Type UBTTask_SetMonsterState::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return EBTNodeResult::Failed;

    AMonsterCharacter* Mon = Cast<AMonsterCharacter>(AICon->GetPawn());
    if (!Mon) return EBTNodeResult::Failed;

    // ���� enum�� �ٲٰ� (������ ��ũ�� ĳ���� �Լ��� ����)
    Mon->SetMonsterState(static_cast<EMonsterState>(TargetState));

    return EBTNodeResult::Succeeded;
}
