#include "BT/BTTask_AttackPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Monster/MonsterCharacter.h"
#include "Data/MonsterDefinition.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

UBTTask_AttackPlayer::UBTTask_AttackPlayer()
{
	NodeName = TEXT("Attack Player");
	bNotifyTick = true;
	bCreateNodeInstance = true;

	// GrabFlow 런타임 상태 초기화
	bUseGrabFlowThisTask = false;
	bSentVictimEvent = false;
	bRequestedGrabSection = false;
	StartTimeForGrab = 0.f;
}

EBTNodeResult::Type UBTTask_AttackPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AI = OwnerComp.GetAIOwner();
	if (!AI) return EBTNodeResult::Failed;

	AMonsterCharacter* MC = Cast<AMonsterCharacter>(AI->GetPawn());
	if (!MC) return EBTNodeResult::Failed;

	USkeletalMeshComponent* Mesh = MC->GetMesh();
	if (!Mesh) return EBTNodeResult::Failed;

	UAnimInstance* Anim = Mesh->GetAnimInstance();
	if (!Anim) return EBTNodeResult::Failed;

	UMonsterDefinition* Def = MC->GetMonsterDef();
	if (!Def) return EBTNodeResult::Failed;

    UAnimMontage* AttackMontage = nullptr;
    ChosenSection = NAME_None;

    bool bPicked = false;
    if (!AttackKey.IsNone())
    {
        // 키로 특정 공격 선택
        bPicked = Def->FindAttackByKey(AttackKey, AttackMontage, ChosenSection);
    }
    else
    {
        // 키 없으면 가중치 랜덤
        bPicked = Def->PickRandomAttack(AttackMontage, ChosenSection);
    }
    if (!bPicked || !AttackMontage) return EBTNodeResult::Failed;


	// 이번 태스크에서 그랩 플로우 활성화 여부 결정
	// AttackKey가 "grab"와 같거나, bEnableGrabFlow가 켜져 있으면 활성화
	bUseGrabFlowThisTask = false;
	if (!AttackKey.IsNone() && AttackKey == ConfigGrabKey) bUseGrabFlowThisTask = true;
	if (bEnableGrabFlow) bUseGrabFlowThisTask = true;

	// 이미 같은 몽타주 재생 중이면 재시작 금지
	if (Anim->Montage_IsPlaying(AttackMontage))
	{
		BoundAnim = Anim;
		CachedMontage = AttackMontage;
		CachedBTC = &OwnerComp;

		if (BoundAnim && BoundAnim != Anim && bBoundDelegate)
		{
			BoundAnim->OnMontageEnded.RemoveDynamic(this, &UBTTask_AttackPlayer::HandleMontageEnded);
			bBoundDelegate = false;
		}

		// 항상 중복 제거 후 유니크 바인딩
		Anim->OnMontageEnded.RemoveDynamic(this, &UBTTask_AttackPlayer::HandleMontageEnded);
		Anim->OnMontageEnded.AddUniqueDynamic(this, &UBTTask_AttackPlayer::HandleMontageEnded);
		bBoundDelegate = true;
		BoundAnim = Anim;

		MontageDuration = AttackMontage->GetPlayLength();
		ElapsedTime = Anim->Montage_GetPosition(AttackMontage);

		ApplyAttackMovementLock(MC);
		AI->StopMovement();

		MC->PushAttackCollision();   // 공격 중 몬스터끼리 Ignore

		// [ADD] Grab 플로우 초기화(이미 재생 중이더라도 신호 대기 가능)
		if (bUseGrabFlowThisTask)
		{
			if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
			{
				if (!BB_GrabConfirmedKey.IsNone())
				{
					BB->SetValueAsBool(BB_GrabConfirmedKey, false);
				}
			}
			StartTimeForGrab = MC->GetWorld()->GetTimeSeconds();
			bSentVictimEvent = false;
			bRequestedGrabSection = false;
		}
		return EBTNodeResult::InProgress;
	}

	// 새로 재생
	const float PlayedLen = Anim->Montage_Play(AttackMontage, 1.f);
	if (PlayedLen <= 0.f) return EBTNodeResult::Failed;

	BoundAnim = Anim;
	CachedMontage = AttackMontage;
	CachedBTC = &OwnerComp;
	bFinishedByEvent = false;

	MontageDuration = AttackMontage->GetPlayLength();
	ElapsedTime = 0.f;

	if (BoundAnim && BoundAnim != Anim && bBoundDelegate)
	{
		BoundAnim->OnMontageEnded.RemoveDynamic(this, &UBTTask_AttackPlayer::HandleMontageEnded);
		bBoundDelegate = false;
	}

	Anim->OnMontageEnded.RemoveDynamic(this, &UBTTask_AttackPlayer::HandleMontageEnded);
	Anim->OnMontageEnded.AddUniqueDynamic(this, &UBTTask_AttackPlayer::HandleMontageEnded);
	bBoundDelegate = true;
	BoundAnim = Anim;

	ApplyAttackMovementLock(MC);
	AI->StopMovement();

	MC->PushAttackCollision();   // 공격 시작 시 프로필 스위치

	// Grab 플로우 초기화
	if (bUseGrabFlowThisTask)
	{
		if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
		{
			if (!BB_GrabConfirmedKey.IsNone())
			{
				BB->SetValueAsBool(BB_GrabConfirmedKey, false);
			}
		}
		StartTimeForGrab = MC->GetWorld()->GetTimeSeconds();
		bSentVictimEvent = false;
		bRequestedGrabSection = false;
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_AttackPlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	ElapsedTime += DeltaSeconds;

	// 에지 케이스: 재생이 멈췄는데 이벤트를 못 받은 경우
	if (BoundAnim && CachedMontage && !BoundAnim->Montage_IsPlaying(CachedMontage))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}

	// =========================
	// Grab 플로우 신호 감시
	// =========================
	if (bUseGrabFlowThisTask)
	{
		UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
		bool bGrabConfirmed = false;
		if (BB && !BB_GrabConfirmedKey.IsNone())
		{
			bGrabConfirmed = BB->GetValueAsBool(BB_GrabConfirmedKey);
		}

		// 타임아웃 처리
		if (GrabTimeoutSeconds > 0.f)
		{
			AAIController* AI = OwnerComp.GetAIOwner();
			if (AI && AI->GetWorld())
			{
				float Now = AI->GetWorld()->GetTimeSeconds();
				if (Now - StartTimeForGrab >= GrabTimeoutSeconds)
				{
					// [NOTE] 타임아웃 시 실패로 반환하거나, 그냥 일반 공격 마무리로 넘겨도 됩니다.
					FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
					return;
				}
			}
		}

		if (bGrabConfirmed)
		{
			// 피해자 이벤트(피해자 몽타주 시작) 1회 전송
			if (!bSentVictimEvent)
			{
				ACharacter* Victim = GetVictim(OwnerComp);
				TrySendVictimStartEvent(Victim);
				bSentVictimEvent = true;
			}

			// 보스 몽타주를 Grab 섹션으로 점프 (한 번만)
			if (!bRequestedGrabSection)
			{
				ACharacter* Boss = nullptr;
				if (AAIController* AI = OwnerComp.GetAIOwner())
				{
					Boss = Cast<ACharacter>(AI->GetPawn());
				}
				JumpToGrabSectionIfNeeded(Boss);
				bRequestedGrabSection = true;
			}
		}
	}
}

void UBTTask_AttackPlayer::HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != CachedMontage) return;

	bFinishedByEvent = true;

	if (CachedBTC)
	{
		FinishLatentTask(*CachedBTC, bInterrupted ? EBTNodeResult::Failed : EBTNodeResult::Succeeded);
	}
}

void UBTTask_AttackPlayer::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	// Dynamic 언바인딩
	if (BoundAnim && bBoundDelegate)
	{
		BoundAnim->OnMontageEnded.RemoveDynamic(this, &UBTTask_AttackPlayer::HandleMontageEnded);
	}
	bBoundDelegate = false;

	// 이동/RVO 복구 및 충돌 복원
	AAIController* AI = OwnerComp.GetAIOwner();
	AMonsterCharacter* MC = nullptr;
	if (AI)
	{
		if (APawn* P = AI->GetPawn())
		{
			if (ACharacter* C = Cast<ACharacter>(P))
			{
				RestoreMovementFromLock(C);
				MC = Cast<AMonsterCharacter>(C);
			}
		}
	}
	if (MC)
	{
		MC->PopAttackCollision();   // 공격 종료 시 원복
	}

	BoundAnim = nullptr;
	CachedMontage = nullptr;
	CachedBTC = nullptr;

	MontageDuration = 0.f;
	ElapsedTime = 0.f;
	bFinishedByEvent = false;

	// Grab 플로우 상태 초기화
	bUseGrabFlowThisTask = false;
	bSentVictimEvent = false;
	bRequestedGrabSection = false;
	StartTimeForGrab = 0.f;

}

void UBTTask_AttackPlayer::ApplyAttackMovementLock(ACharacter* C)
{
	if (!C || bLockApplied) return;

	if (UCharacterMovementComponent* Mv = C->GetCharacterMovement())
	{
		bPrevUseRVO = Mv->bUseRVOAvoidance;
		bPrevOrientToMovement = Mv->bOrientRotationToMovement;
		bPrevUseControllerYaw = C->bUseControllerRotationYaw;

		Mv->StopMovementImmediately();
		Mv->bUseRVOAvoidance = false;
		Mv->bOrientRotationToMovement = false;
		C->bUseControllerRotationYaw = true;
	}
	bLockApplied = true;
}

void UBTTask_AttackPlayer::RestoreMovementFromLock(ACharacter* C)
{
	if (!C || !bLockApplied) return;

	if (UCharacterMovementComponent* Mv = C->GetCharacterMovement())
	{
		Mv->bUseRVOAvoidance = bPrevUseRVO;
		Mv->bOrientRotationToMovement = bPrevOrientToMovement;
		C->bUseControllerRotationYaw = bPrevUseControllerYaw;

		if (Mv->MovementMode == MOVE_None)
		{
			Mv->SetMovementMode(MOVE_Walking);
		}
	}
	bLockApplied = false;
}

EBTNodeResult::Type UBTTask_AttackPlayer::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 1) 델리게이트 해제
	if (BoundAnim && bBoundDelegate)
	{
		BoundAnim->OnMontageEnded.RemoveDynamic(this, &UBTTask_AttackPlayer::HandleMontageEnded);
	}
	bBoundDelegate = false;

	// 2) 이동/RVO/회전 복구 + 충돌 프로필 복원
	if (AAIController* AI = OwnerComp.GetAIOwner())
	{
		if (ACharacter* C = Cast<ACharacter>(AI->GetPawn()))
		{
			RestoreMovementFromLock(C);

			if (AMonsterCharacter* MC = Cast<AMonsterCharacter>(C))
			{
				MC->PopAttackCollision();
			}
		}
	}

	// 3) 캐시 정리
	BoundAnim = nullptr;
	CachedMontage = nullptr;
	CachedBTC = nullptr;
	MontageDuration = 0.f;
	ElapsedTime = 0.f;
	bFinishedByEvent = false;
	bLockApplied = false;

	// Grab 플로우 상태 초기화
	bUseGrabFlowThisTask = false;
	bSentVictimEvent = false;
	bRequestedGrabSection = false;
	StartTimeForGrab = 0.f;

	// 태스크 Abort 결과 반환
	return EBTNodeResult::Aborted;
}


ACharacter* UBTTask_AttackPlayer::GetVictim(UBehaviorTreeComponent& OwnerComp) const
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return nullptr;

	// [NOTE] 프로젝트에서 실제 사용하는 키명에 맞춰 BB_TargetActorKey 변경 가능
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(BB_TargetActorKey));
	return Cast<ACharacter>(Target);
}

void UBTTask_AttackPlayer::TrySendVictimStartEvent(ACharacter* Victim) const
{
	if (!Victim) return;
	if (!VictimStartEventTag.IsValid()) return;

	FGameplayEventData Data;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Victim, VictimStartEventTag, Data);
}

void UBTTask_AttackPlayer::JumpToGrabSectionIfNeeded(ACharacter* Boss) const
{
	if (!Boss || !CachedMontage || BossGrabSectionName.IsNone()) return;
	UAnimInstance* Anim = Boss->GetMesh() ? Boss->GetMesh()->GetAnimInstance() : nullptr;
	if (!Anim) return;

	if (Anim->Montage_IsPlaying(CachedMontage))
	{
		Anim->Montage_JumpToSection(BossGrabSectionName, CachedMontage);
	}
}

bool UBTTask_AttackPlayer::IsMontagePlaying(ACharacter* Boss, UAnimMontage* Montage) const
{
	if (!Boss || !Montage) return false;
	UAnimInstance* Anim = Boss->GetMesh() ? Boss->GetMesh()->GetAnimInstance() : nullptr;
	if (!Anim) return false;
	return Anim->Montage_IsPlaying(Montage);
}