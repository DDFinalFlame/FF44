#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateTarget.generated.h"

UCLASS()
class MONSTERAIPLUGIN_API UBTService_UpdateTarget : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_UpdateTarget();

protected:
	// BT/BB 에셋에서 키 타입/존재 여부 확인
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	// 주기적으로 실행
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

public:
	/** 추적 시작 거리 */
	UPROPERTY(EditAnywhere, Category = "Sense")
	float DetectDistance;

	/** 공격 가능 거리 */
	UPROPERTY(EditAnywhere, Category = "Sense")
	float AttackDistance;

	/** 타겟 액터(Object: Actor) */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector TargetActorKey;

	/** 공격 가능 여부(Bool) */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector CanAttackKey;

	/** 플레이어 인덱스 (싱글이면 0 고정) */
	UPROPERTY(EditAnywhere, Category = "Sense")
	int32 PlayerIndex;

	UPROPERTY(EditAnywhere, Category = "Blackboard|Optional")
	struct FBlackboardKeySelector HasLineOfSightKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard|Optional")
	struct FBlackboardKeySelector LastKnownLocationKey;  

	UPROPERTY(EditAnywhere, Category = "Blackboard|Optional")
	struct FBlackboardKeySelector DistanceToTargetKey;   

	UPROPERTY(EditAnywhere, Category = "Blackboard|Optional")
	struct FBlackboardKeySelector NextAttackTimeKey;

	UPROPERTY(EditAnywhere, Category = "Sense|FOV")
	float FOVDegrees = 80.f;              // 전체 시야각(예: 120도)

	UPROPERTY(EditAnywhere, Category = "Sense|FOV")
	bool bUsePlanarFOV = true;             // true면 Z 무시(수평면 기준 각도)
};
