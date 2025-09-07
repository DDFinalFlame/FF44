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

	// GrabFlow ��Ÿ�� ���� �ʱ�ȭ
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
        // Ű�� Ư�� ���� ����
        bPicked = Def->FindAttackByKey(AttackKey, AttackMontage, ChosenSection);
    }
    else
    {
        // Ű ������ ����ġ ����
        bPicked = Def->PickRandomAttack(AttackMontage, ChosenSection);
    }
    if (!bPicked || !AttackMontage) return EBTNodeResult::Failed;


	// �̹� �½�ũ���� �׷� �÷ο� Ȱ��ȭ ���� ����
	// AttackKey�� "grab"�� ���ų�, bEnableGrabFlow�� ���� ������ Ȱ��ȭ
	bUseGrabFlowThisTask = false;
	if (!AttackKey.IsNone() && AttackKey == ConfigGrabKey) bUseGrabFlowThisTask = true;
	if (bEnableGrabFlow) bUseGrabFlowThisTask = true;

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

		// [ADD] Grab �÷ο� �ʱ�ȭ(�̹� ��� ���̴��� ��ȣ ��� ����)
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

	// Grab �÷ο� �ʱ�ȭ
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

	// ���� ���̽�: ����� ����µ� �̺�Ʈ�� �� ���� ���
	if (BoundAnim && CachedMontage && !BoundAnim->Montage_IsPlaying(CachedMontage))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}

	// =========================
	// Grab �÷ο� ��ȣ ����
	// =========================
	if (bUseGrabFlowThisTask)
	{
		UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
		bool bGrabConfirmed = false;
		if (BB && !BB_GrabConfirmedKey.IsNone())
		{
			bGrabConfirmed = BB->GetValueAsBool(BB_GrabConfirmedKey);
		}

		// Ÿ�Ӿƿ� ó��
		if (GrabTimeoutSeconds > 0.f)
		{
			AAIController* AI = OwnerComp.GetAIOwner();
			if (AI && AI->GetWorld())
			{
				float Now = AI->GetWorld()->GetTimeSeconds();
				if (Now - StartTimeForGrab >= GrabTimeoutSeconds)
				{
					// [NOTE] Ÿ�Ӿƿ� �� ���з� ��ȯ�ϰų�, �׳� �Ϲ� ���� �������� �Ѱܵ� �˴ϴ�.
					FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
					return;
				}
			}
		}

		if (bGrabConfirmed)
		{
			// ������ �̺�Ʈ(������ ��Ÿ�� ����) 1ȸ ����
			if (!bSentVictimEvent)
			{
				ACharacter* Victim = GetVictim(OwnerComp);
				TrySendVictimStartEvent(Victim);
				bSentVictimEvent = true;
			}

			// ���� ��Ÿ�ָ� Grab �������� ���� (�� ����)
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

	// Grab �÷ο� ���� �ʱ�ȭ
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

	// Grab �÷ο� ���� �ʱ�ȭ
	bUseGrabFlowThisTask = false;
	bSentVictimEvent = false;
	bRequestedGrabSection = false;
	StartTimeForGrab = 0.f;

	// �½�ũ Abort ��� ��ȯ
	return EBTNodeResult::Aborted;
}


ACharacter* UBTTask_AttackPlayer::GetVictim(UBehaviorTreeComponent& OwnerComp) const
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return nullptr;

	// [NOTE] ������Ʈ���� ���� ����ϴ� Ű�� ���� BB_TargetActorKey ���� ����
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