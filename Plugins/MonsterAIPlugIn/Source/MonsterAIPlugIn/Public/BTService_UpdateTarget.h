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
	// BT/BB ���¿��� Ű Ÿ��/���� ���� Ȯ��
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	// �ֱ������� ����
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

public:
	/** ���� ���� �Ÿ� */
	UPROPERTY(EditAnywhere, Category = "Sense")
	float DetectDistance;

	/** ���� ���� �Ÿ� */
	UPROPERTY(EditAnywhere, Category = "Sense")
	float AttackDistance;

	/** Ÿ�� ����(Object: Actor) */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector TargetActorKey;

	/** ���� ���� ����(Bool) */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector CanAttackKey;

	/** �÷��̾� �ε��� (�̱��̸� 0 ����) */
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
	float FOVDegrees = 80.f;              // ��ü �þ߰�(��: 120��)

	UPROPERTY(EditAnywhere, Category = "Sense|FOV")
	bool bUsePlanarFOV = true;             // true�� Z ����(����� ���� ����)
};
