#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
//#include "BehaviorTree/Blackboard/BlackboardKeySelector.h"
#include "BTTask_LookAtTarget.generated.h"

UCLASS(Blueprintable)
class MONSTERAIPLUGIN_API UBTTask_LookAtTarget : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_LookAtTarget();

    // �ٶ� Ÿ��(Actor/Object). ���� Actor Ű.
    UPROPERTY(EditAnywhere, Category = "LookAt")
    FBlackboardKeySelector TargetActorKey;

    // �ʴ� ȸ�� �ӵ�(��/��)
    UPROPERTY(EditAnywhere, Category = "LookAt", meta = (ClampMin = "1.0"))
    float YawSpeedDeg;

    // �� ���� ���Ϸ� ���ĵǸ� ���� �Ϸ�� �Ǵ�(��)
    UPROPERTY(EditAnywhere, Category = "LookAt", meta = (ClampMin = "0.0"))
    float AcceptAngleDeg;

    // ���ĵǸ� Task�� ������(������ ������ Abort�� ������ ��� �ٶ�)
    UPROPERTY(EditAnywhere, Category = "LookAt")
    bool bFinishWhenAligned;

    // ������ġ Ÿ�Ӿƿ�(��, 0�̸� ��Ȱ��)
    UPROPERTY(EditAnywhere, Category = "LookAt", meta = (ClampMin = "0.0"))
    float TimeoutSeconds;

    // AIController Focus ��� ���� (��� �� Perception/Anim ��� ���� ����)
    UPROPERTY(EditAnywhere, Category = "LookAt")
    bool bUseAIFocus;

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

private:
    float StartTime;
};