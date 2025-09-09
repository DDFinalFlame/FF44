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

	// --- 재생 상태/이벤트 관리용 ---
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

	// (옵션) 공격 중 이동/회전/RVO 잠깐 고정
	void ApplyAttackMovementLock(class ACharacter* C);
	void RestoreMovementFromLock(class ACharacter* C);

	bool bPrevUseRVO;
	bool bPrevOrientToMovement;
	bool bPrevUseControllerYaw;
	bool bLockApplied;

public:
	// "grab" 키일 때 그랩 플로우를 자동 활성화할지 제어
	UPROPERTY(EditAnywhere, Category = "GrabFlow")
	FName ConfigGrabKey = TEXT("grab");

	// 강제로 그랩 플로우를 켤 수도 있음(기본은 false).
	UPROPERTY(EditAnywhere, Category = "GrabFlow")
	bool bEnableGrabFlow = false;

	// 무기 히트 콜백에서 올려줄 신호(BB Bool). 예: "bGrabConfirmed"
	UPROPERTY(EditAnywhere, Category = "GrabFlow")
	FName BB_GrabConfirmedKey = TEXT("bGrabConfirmed");

	// 피해자(플레이어)에게 보낼 이벤트 태그(이걸 수신해 피해자 몽타주 시작)
	UPROPERTY(EditAnywhere, Category = "GrabFlow")
	FGameplayTag VictimStartEventTag;

	// 보스 몽타주에서 점프할 섹션 이름(들기/잡기 섹션). 예: "Grab"
	UPROPERTY(EditAnywhere, Category = "GrabFlow")
	FName BossGrabSectionName = TEXT("Grab");

	// Grab 신호 대기 타임아웃(초). 0이면 무제한
	UPROPERTY(EditAnywhere, Category = "GrabFlow")
	float GrabTimeoutSeconds = 3.0f;

	// 타깃 액터가 들어있는 BB 키(ANS/정렬에서 동일 대상 사용). 예: "TargetActor"
	UPROPERTY(EditAnywhere, Category = "GrabFlow")
	FName BB_TargetActorKey = TEXT("TargetActor");

private:
	// 이번 태스크에서 실제로 그랩 플로우를 쓸지(AttackKey/설정에 따라 결정)
	bool bUseGrabFlowThisTask = false;

	// Grab 플로우 런타임 상태
	bool bSentVictimEvent = false;
	bool bRequestedGrabSection = false;
	float StartTimeForGrab = 0.f;

	// 유틸들
	class ACharacter* GetVictim(UBehaviorTreeComponent& OwnerComp) const;
	void TrySendVictimStartEvent(class ACharacter* Victim) const;
	void JumpToGrabSectionIfNeeded(class ACharacter* Boss) const;
	bool IsMontagePlaying(class ACharacter* Boss, UAnimMontage* Montage) const;
};
