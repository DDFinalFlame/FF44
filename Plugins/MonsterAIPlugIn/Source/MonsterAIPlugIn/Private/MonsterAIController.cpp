// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"

AMonsterAIController::AMonsterAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BTAsset(TEXT("/MonsterAIPlugIn/BT_MonsterAI.BT_MonsterAI"));
	if (BTAsset.Succeeded())
	{
		BehaviorTreeAsset = BTAsset.Object;
	}
}

void AMonsterAIController::BeginPlay()
{
	Super::BeginPlay();


}

void AMonsterAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateBlackboardKeys();
}

void AMonsterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	Super::OnPossess(InPawn);

	if (BehaviorTreeAsset)
	{
		UBlackboardComponent* BBComp = nullptr;
		// BT�� �����ϴ� ������� �׻� �ʱ�ȭ
		UseBlackboard(BehaviorTreeAsset->BlackboardAsset, BBComp);
		BlackboardComponent = BBComp; // ����� ĳ��(�ִٸ�)

		const bool bOK = RunBehaviorTree(BehaviorTreeAsset);
		if (!bOK)
		{
			UE_LOG(LogTemp, Warning, TEXT("RunBehaviorTree failed on %s"), *GetName());
		}
	}
}


void AMonsterAIController::UpdateBlackboardKeys()
{
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!Player) return;

	APawn* SelfPawn = GetPawn();
	if (!SelfPawn) return;

	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		float Distance = FVector::Dist(Player->GetActorLocation(), SelfPawn->GetActorLocation());

		const float DetectDistance = 1500.f;  // ���� ���� �Ÿ�
		const float AttackDistance = 800.f;   // ���� ���� �Ÿ�

		if (Distance < DetectDistance)
		{
			// ���� ���� ���̸� Ÿ�� ����
			BB->SetValueAsObject(TEXT("TargetActor"), Player);
			BB->SetValueAsBool(TEXT("CanAttack"), Distance < AttackDistance);
		}
		else
		{
			// ���� ���̸� Ÿ�� �ʱ�ȭ (-> ������ ��ȯ��)
			BB->ClearValue(TEXT("TargetActor"));
			BB->SetValueAsBool(TEXT("CanAttack"), false);
		}
	}
}