#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateTarget.generated.h"

/**
 * Perception이 세팅한 TargetActor/HasLineOfSight/LastKnownLocation을 "읽어"
 * DistanceToTarget, CanAttack 등 전투 보조 값만 갱신하는 서비스
 */
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
	/** 타깃 액터(Object: Actor) ? Perception이 세팅, 서비스는 "읽기 전용" */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector TargetActorKey;

	/** 공격 가능 여부(Bool) ? 서비스가 계산하여 세팅 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector CanAttackKey;

	/** 타깃까지의 거리(Float) ? 서비스가 계산하여 세팅 */
	UPROPERTY(EditAnywhere, Category = "Blackboard|Optional")
	struct FBlackboardKeySelector DistanceToTargetKey;

	/** 공격 사거리(Float) ? BB에 있으면 우선, 없으면 기본값 사용 */
	UPROPERTY(EditAnywhere, Category = "Blackboard|Optional")
	struct FBlackboardKeySelector AttackDistanceKey;

	/** Perception이 세팅한 시야 여부를 읽어 CanAttack 조건에 포함할지 여부 */
	UPROPERTY(EditAnywhere, Category = "Sense")
	bool bRequireLOSForCanAttack = true;

	/** Perception이 세팅한 HasLineOfSight(Bool) ? 읽기 전용(선택) */
	UPROPERTY(EditAnywhere, Category = "Blackboard|Optional")
	struct FBlackboardKeySelector HasLineOfSightKey;

	/** 기본 공격 사거리 (BB 없을 때 사용) */
	UPROPERTY(EditAnywhere, Category = "Defaults")
	float DefaultAttackDistance = 200.f;
};