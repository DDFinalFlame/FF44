// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Animation/AnimMontage.h"
#include "BTTask_AttackPlayer.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class MONSTERAIPLUGIN_API UBTTask_AttackPlayer : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_AttackPlayer();

	// --- ��� ����/�̺�Ʈ ������ ---
	UPROPERTY() UAnimInstance* BoundAnim;
	UPROPERTY() UAnimMontage* CachedMontage;
	UPROPERTY() class UBehaviorTreeComponent* CachedBTC;

	bool bFinishedByEvent = false;
	bool bBoundDelegate = false;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	UFUNCTION() void HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted);

private:
	float MontageDuration;
	float ElapsedTime;

	// (�ɼ�) ���� �� �̵�/ȸ��/RVO ��� ����
	void ApplyAttackMovementLock(class ACharacter* C);
	void RestoreMovementFromLock(class ACharacter* C);

	bool bPrevUseRVO;
	bool bPrevOrientToMovement;
	bool bPrevUseControllerYaw;
	bool bLockApplied;
};
