// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_RestoreBossState.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_RestoreBossState::UBTTask_RestoreBossState()
{
    NodeName = TEXT("Restore BossState From Prev");
    // 기본 키 이름
    BossStateKey.SelectedKeyName = TEXT("BossState");
    PrevBossStateKey.SelectedKeyName = TEXT("PrevBossState");
}

void UBTTask_RestoreBossState::InitializeFromAsset(UBehaviorTree& Asset)
{
    Super::InitializeFromAsset(Asset);

    if (UBlackboardData* BBAsset = GetBlackboardAsset())
    {
        // Enum 키로 지정되어 있어야 함 (필터는 선택 사항)
        BossStateKey.ResolveSelectedKey(*BBAsset);
        PrevBossStateKey.ResolveSelectedKey(*BBAsset);
    }
}


EBTNodeResult::Type UBTTask_RestoreBossState::ExecuteTask(UBehaviorTreeComponent& _OwnerComp, uint8* _NodeMemory)
{
    UBlackboardComponent* BB = _OwnerComp.GetBlackboardComponent();
    if (!BB) return EBTNodeResult::Failed;

    if (!BossStateKey.SelectedKeyType || !PrevBossStateKey.SelectedKeyType)
        return EBTNodeResult::Failed;

    // Prev 값을 읽어 BossState 로 복구
    uint8 Prev = BB->GetValueAsEnum(PrevBossStateKey.SelectedKeyName);

    // Prev 값이 아직 안 채워졌다면(프로젝트 정책에 따라 0도 유효할 수 있음) 그냥 성공 처리
    // 필요하면 여기에 유효성 체크 로직 추가
    BB->SetValueAsEnum(BossStateKey.SelectedKeyName, Prev);

    if (bClearPrevAfterRestore)
    {
        // 0으로 리셋(원하시면 특정 기본값으로 바꾸세요)
        BB->SetValueAsEnum(PrevBossStateKey.SelectedKeyName, 0);
    }

    return EBTNodeResult::Succeeded;
}
