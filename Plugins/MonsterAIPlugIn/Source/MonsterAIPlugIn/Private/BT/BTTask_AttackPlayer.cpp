#include "BT/BTTask_AttackPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Monster/MonsterCharacter.h"
#include "Data/MonsterDefinition.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTTask_AttackPlayer::UBTTask_AttackPlayer()
{
	NodeName = TEXT("Attack Player");
	bNotifyTick = true;
	bCreateNodeInstance = true;
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
        // Ű�� Ư�� ���� ����
        bPicked = Def->FindAttackByKey(AttackKey, AttackMontage, ChosenSection);
    }
    else
    {
        // Ű ������ ����ġ ����
        bPicked = Def->PickRandomAttack(AttackMontage, ChosenSection);
    }
    if (!bPicked || !AttackMontage) return EBTNodeResult::Failed;

	// �̹� ���� ��Ÿ�� ��� ���̸� ����� ����
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

		// �׻� �ߺ� ���� �� ����ũ ���ε�
		Anim->OnMontageEnded.RemoveDynamic(this, &UBTTask_AttackPlayer::HandleMontageEnded);
		Anim->OnMontageEnded.AddUniqueDynamic(this, &UBTTask_AttackPlayer::HandleMontageEnded);
		bBoundDelegate = true;
		BoundAnim = Anim;

		MontageDuration = AttackMontage->GetPlayLength();
		ElapsedTime = Anim->Montage_GetPosition(AttackMontage);

		ApplyAttackMovementLock(MC);
		AI->StopMovement();

		MC->PushAttackCollision();   // ���� �� ���ͳ��� Ignore
		return EBTNodeResult::InProgress;
	}

	// ���� ���
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

	MC->PushAttackCollision();   // ���� ���� �� ������ ����ġ
	return EBTNodeResult::InProgress;
}

void UBTTask_AttackPlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	ElapsedTime += DeltaSeconds;

	// ���� ���̽�: ����� ����µ� �̺�Ʈ�� �� ���� ���
	if (BoundAnim && CachedMontage && !BoundAnim->Montage_IsPlaying(CachedMontage))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
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
	// Dynamic ����ε�
	if (BoundAnim && bBoundDelegate)
	{
		BoundAnim->OnMontageEnded.RemoveDynamic(this, &UBTTask_AttackPlayer::HandleMontageEnded);
	}
	bBoundDelegate = false;

	// �̵�/RVO ���� �� �浹 ����
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
		MC->PopAttackCollision();   // ���� ���� �� ����
	}

	BoundAnim = nullptr;
	CachedMontage = nullptr;
	CachedBTC = nullptr;

	MontageDuration = 0.f;
	ElapsedTime = 0.f;
	bFinishedByEvent = false;
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
	// 1) ��������Ʈ ����
	if (BoundAnim && bBoundDelegate)
	{
		BoundAnim->OnMontageEnded.RemoveDynamic(this, &UBTTask_AttackPlayer::HandleMontageEnded);
	}
	bBoundDelegate = false;

	// 2) �̵�/RVO/ȸ�� ���� + �浹 ������ ����
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

	// 3) ĳ�� ����
	BoundAnim = nullptr;
	CachedMontage = nullptr;
	CachedBTC = nullptr;
	MontageDuration = 0.f;
	ElapsedTime = 0.f;
	bFinishedByEvent = false;
	bLockApplied = false;

	// �½�ũ Abort ��� ��ȯ
	return EBTNodeResult::Aborted;
}