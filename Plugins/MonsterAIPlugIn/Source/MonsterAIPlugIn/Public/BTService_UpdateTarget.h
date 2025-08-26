#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateTarget.generated.h"

/**
 * Perception�� ������ TargetActor/HasLineOfSight/LastKnownLocation�� "�о�"
 * DistanceToTarget, CanAttack �� ���� ���� ���� �����ϴ� ����
 */
UCLASS()
class MONSTERAIPLUGIN_API UBTService_UpdateTarget : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_UpdateTarget();

protected:
	// BT/BB ���¿��� Ű Ÿ��/���� ���� Ȯ��
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	// �ֱ������� ����
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

public:
	/** Ÿ�� ����(Object: Actor) ? Perception�� ����, ���񽺴� "�б� ����" */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector TargetActorKey;

	/** ���� ���� ����(Bool) ? ���񽺰� ����Ͽ� ���� */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector CanAttackKey;

	/** Ÿ������� �Ÿ�(Float) ? ���񽺰� ����Ͽ� ���� */
	UPROPERTY(EditAnywhere, Category = "Blackboard|Optional")
	struct FBlackboardKeySelector DistanceToTargetKey;

	/** ���� ��Ÿ�(Float) ? BB�� ������ �켱, ������ �⺻�� ��� */
	UPROPERTY(EditAnywhere, Category = "Blackboard|Optional")
	struct FBlackboardKeySelector AttackDistanceKey;

	/** Perception�� ������ �þ� ���θ� �о� CanAttack ���ǿ� �������� ���� */
	UPROPERTY(EditAnywhere, Category = "Sense")
	bool bRequireLOSForCanAttack = true;

	/** Perception�� ������ HasLineOfSight(Bool) ? �б� ����(����) */
	UPROPERTY(EditAnywhere, Category = "Blackboard|Optional")
	struct FBlackboardKeySelector HasLineOfSightKey;

	/** �⺻ ���� ��Ÿ� (BB ���� �� ���) */
	UPROPERTY(EditAnywhere, Category = "Defaults")
	float DefaultAttackDistance = 200.f;
};