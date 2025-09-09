// Fill out your copyright notice in the Description page of Project Settings.


#include "BT/BTTask_AmbushAttack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monster/MonsterCharacter.h"

EBTNodeResult::Type UBTTask_AmbushAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return EBTNodeResult::Failed;

    APawn* Pawn = AICon->GetPawn();
    if (!Pawn) return EBTNodeResult::Failed;

    AMonsterCharacter* Monster = Cast<AMonsterCharacter>(Pawn);
    if (!Monster) return EBTNodeResult::Failed;

    // (�ɼ�) �߰� ������ ���⼭ �� �� �� Ȯ���ϰ� ������:
    // if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
    // {
    //     UObject* Target = BB->GetValueAsObject(TEXT("TargetActor"));
    //     if (!Target) return EBTNodeResult::Failed;
    // }

    // 1) ���� ���� (GAS �ɷ� ����)
    Monster->Attack();

    // 2) ��� ���� �� �Ϲ� ���� �շ�
    //    (�� �Լ��� ���ο��� State=CombatReady�� �ٲٰ� BB�� ����ȭ�Ѵٰ� ����)
    Monster->FinishAmbush();

    return EBTNodeResult::Succeeded;
}