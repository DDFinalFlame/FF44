// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_AttackPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "MonsterCharacter.h"
#include "MonsterDefinition.h"
#include "Animation/AnimInstance.h"

UBTTask_AttackPlayer::UBTTask_AttackPlayer()
{
	NodeName = TEXT("Attack Player");
	bNotifyTick = true; // TickTask 사용 가능하게 설정
	MontageDuration = 0.0f;
	ElapsedTime = 0.0f;
}

EBTNodeResult::Type UBTTask_AttackPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AI = OwnerComp.GetAIOwner();
    if (!AI) return EBTNodeResult::Failed;

    AMonsterCharacter* MC = Cast<AMonsterCharacter>(AI->GetPawn());
    if (!MC) return EBTNodeResult::Failed;

    UAnimInstance* Anim = MC->GetMesh() ? MC->GetMesh()->GetAnimInstance() : nullptr;
    if (!Anim) return EBTNodeResult::Failed;

    UMonsterDefinition* Def = MC->GetMonsterDef();     // ← 캐릭터에서 DA 꺼내오기
    if (!Def) return EBTNodeResult::Failed;

    if (!Def->AttackMontage.IsValid()) Def->AttackMontage.LoadSynchronous();
    UAnimMontage* AttackMontage = Def->AttackMontage.Get();
    if (!AttackMontage) return EBTNodeResult::Failed;

    const float PlayedLen = Anim->Montage_Play(AttackMontage, 1.f);
    if (PlayedLen <= 0.f) return EBTNodeResult::Failed;

    MontageDuration = AttackMontage->GetPlayLength();  // 섹션 안 쓸 경우
    ElapsedTime = 0.f;
    return EBTNodeResult::InProgress;
}

void UBTTask_AttackPlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	ElapsedTime += DeltaSeconds;
	if (ElapsedTime >= MontageDuration)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

