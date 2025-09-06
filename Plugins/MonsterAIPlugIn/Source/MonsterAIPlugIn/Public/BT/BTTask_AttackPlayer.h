// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Animation/AnimMontage.h"
#include "GameplayTagContainer.h"
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
	UPROPERTY() 
	UAnimInstance* BoundAnim;
	UPROPERTY() 
	UAnimMontage* CachedMontage;
	UPROPERTY() 
	class UBehaviorTreeComponent* CachedBTC;
	UPROPERTY(EditAnywhere, Category = "Attack")	
	FName AttackKey;
	
	FName ChosenSection = NAME_None;

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

public:
	// "grab" Ű�� �� �׷� �÷ο츦 �ڵ� Ȱ��ȭ���� ����
	UPROPERTY(EditAnywhere, Category = "GrabFlow")
	FName ConfigGrabKey = TEXT("grab");

	// ������ �׷� �÷ο츦 �� ���� ����(�⺻�� false).
	UPROPERTY(EditAnywhere, Category = "GrabFlow")
	bool bEnableGrabFlow = false;

	// ���� ��Ʈ �ݹ鿡�� �÷��� ��ȣ(BB Bool). ��: "bGrabConfirmed"
	UPROPERTY(EditAnywhere, Category = "GrabFlow")
	FName BB_GrabConfirmedKey = TEXT("bGrabConfirmed");

	// ������(�÷��̾�)���� ���� �̺�Ʈ �±�(�̰� ������ ������ ��Ÿ�� ����)
	UPROPERTY(EditAnywhere, Category = "GrabFlow")
	FGameplayTag VictimStartEventTag;

	// ���� ��Ÿ�ֿ��� ������ ���� �̸�(���/��� ����). ��: "Grab"
	UPROPERTY(EditAnywhere, Category = "GrabFlow")
	FName BossGrabSectionName = TEXT("Grab");

	// Grab ��ȣ ��� Ÿ�Ӿƿ�(��). 0�̸� ������
	UPROPERTY(EditAnywhere, Category = "GrabFlow")
	float GrabTimeoutSeconds = 3.0f;

	// Ÿ�� ���Ͱ� ����ִ� BB Ű(ANS/���Ŀ��� ���� ��� ���). ��: "TargetActor"
	UPROPERTY(EditAnywhere, Category = "GrabFlow")
	FName BB_TargetActorKey = TEXT("TargetActor");

private:
	// �̹� �½�ũ���� ������ �׷� �÷ο츦 ����(AttackKey/������ ���� ����)
	bool bUseGrabFlowThisTask = false;

	// Grab �÷ο� ��Ÿ�� ����
	bool bSentVictimEvent = false;
	bool bRequestedGrabSection = false;
	float StartTimeForGrab = 0.f;

	// ��ƿ��
	class ACharacter* GetVictim(UBehaviorTreeComponent& OwnerComp) const;
	void TrySendVictimStartEvent(class ACharacter* Victim) const;
	void JumpToGrabSectionIfNeeded(class ACharacter* Boss) const;
	bool IsMontagePlaying(class ACharacter* Boss, UAnimMontage* Montage) const;
};
