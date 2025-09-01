// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_RestoreBossState.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_RestoreBossState::UBTTask_RestoreBossState()
{
    NodeName = TEXT("Restore BossState From Prev");
    // �⺻ Ű �̸�
    BossStateKey.SelectedKeyName = TEXT("BossState");
    PrevBossStateKey.SelectedKeyName = TEXT("PrevBossState");
}

void UBTTask_RestoreBossState::InitializeFromAsset(UBehaviorTree& Asset)
{
    Super::InitializeFromAsset(Asset);

    if (UBlackboardData* BBAsset = GetBlackboardAsset())
    {
        // Enum Ű�� �����Ǿ� �־�� �� (���ʹ� ���� ����)
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

    // Prev ���� �о� BossState �� ����
    uint8 Prev = BB->GetValueAsEnum(PrevBossStateKey.SelectedKeyName);

    // Prev ���� ���� �� ä�����ٸ�(������Ʈ ��å�� ���� 0�� ��ȿ�� �� ����) �׳� ���� ó��
    // �ʿ��ϸ� ���⿡ ��ȿ�� üũ ���� �߰�
    BB->SetValueAsEnum(BossStateKey.SelectedKeyName, Prev);

    if (bClearPrevAfterRestore)
    {
        // 0���� ����(���Ͻø� Ư�� �⺻������ �ٲټ���)
        BB->SetValueAsEnum(PrevBossStateKey.SelectedKeyName, 0);
    }

    return EBTNodeResult::Succeeded;
}
