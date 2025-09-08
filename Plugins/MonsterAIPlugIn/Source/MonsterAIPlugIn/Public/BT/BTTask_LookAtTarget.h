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

    // 바라볼 타겟(Actor/Object). 보통 Actor 키.
    UPROPERTY(EditAnywhere, Category = "LookAt")
    FBlackboardKeySelector TargetActorKey;

    // 초당 회전 속도(도/초)
    UPROPERTY(EditAnywhere, Category = "LookAt", meta = (ClampMin = "1.0"))
    float YawSpeedDeg;

    // 이 각도 이하로 정렬되면 정렬 완료로 판단(도)
    UPROPERTY(EditAnywhere, Category = "LookAt", meta = (ClampMin = "0.0"))
    float AcceptAngleDeg;

    // 정렬되면 Task를 끝낼지(끝내지 않으면 Abort될 때까지 계속 바라봄)
    UPROPERTY(EditAnywhere, Category = "LookAt")
    bool bFinishWhenAligned;

    // 안전장치 타임아웃(초, 0이면 비활성)
    UPROPERTY(EditAnywhere, Category = "LookAt", meta = (ClampMin = "0.0"))
    float TimeoutSeconds;

    // AIController Focus 사용 여부 (사용 시 Perception/Anim 등과 연동 유리)
    UPROPERTY(EditAnywhere, Category = "LookAt")
    bool bUseAIFocus;

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

private:
    float StartTime;
};