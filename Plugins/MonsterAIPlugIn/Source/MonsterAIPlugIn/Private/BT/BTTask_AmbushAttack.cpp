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

    // (옵션) 발견 조건을 여기서 한 번 더 확인하고 싶으면:
    // if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
    // {
    //     UObject* Target = BB->GetValueAsObject(TEXT("TargetActor"));
    //     if (!Target) return EBTNodeResult::Failed;
    // }

    // 1) 공격 개시 (GAS 능력 실행)
    Monster->Attack();

    // 2) 기습 종료 → 일반 루프 합류
    //    (이 함수는 내부에서 State=CombatReady로 바꾸고 BB와 동기화한다고 가정)
    Monster->FinishAmbush();

    return EBTNodeResult::Succeeded;
}