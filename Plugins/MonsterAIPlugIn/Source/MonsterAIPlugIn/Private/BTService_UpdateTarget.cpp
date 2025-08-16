#include "BTService_UpdateTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

UBTService_UpdateTarget::UBTService_UpdateTarget()
{
	NodeName = TEXT("Update Target & CanAttack");

	// 서비스 호출 간격 (에디터에서도 조정 가능)
	Interval = 0.2f;
	RandomDeviation = 0.0f;

	DetectDistance = 1500.f;
	AttackDistance = 200.f;
	PlayerIndex = 0;

	bNotifyBecomeRelevant = false; 
	bNotifyTick = true;
}


void UBTService_UpdateTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	UBlackboardData* BBAsset = GetBlackboardAsset();
	if (!BBAsset) return;

	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateTarget, TargetActorKey), AActor::StaticClass());
	CanAttackKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateTarget, CanAttackKey));
	HasLineOfSightKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateTarget, HasLineOfSightKey));
	LastKnownLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateTarget, LastKnownLocationKey));
	DistanceToTargetKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateTarget, DistanceToTargetKey));
	NextAttackTimeKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateTarget, NextAttackTimeKey));

	TargetActorKey.ResolveSelectedKey(*BBAsset);
	CanAttackKey.ResolveSelectedKey(*BBAsset);
	HasLineOfSightKey.ResolveSelectedKey(*BBAsset);
	LastKnownLocationKey.ResolveSelectedKey(*BBAsset);
	DistanceToTargetKey.ResolveSelectedKey(*BBAsset);
	NextAttackTimeKey.ResolveSelectedKey(*BBAsset);
}

void UBTService_UpdateTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return;

	APawn* SelfPawn = AICon->GetPawn();
	if (!SelfPawn) return;

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(SelfPawn->GetWorld(), PlayerIndex);
	if (!Player) return;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;

	// 거리 갱신
	const FVector SelfLoc = SelfPawn->GetActorLocation();
	const FVector TargetLoc = Player->GetActorLocation();
	const float   Distance = FVector::Dist(TargetLoc, SelfLoc);

	if (DistanceToTargetKey.SelectedKeyType)
	{
		BB->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, Distance);
	}

	// 타겟/공격 가능 갱신
	if (Distance < DetectDistance)
	{
		BB->SetValueAsObject(TargetActorKey.SelectedKeyName, Player);
		BB->SetValueAsBool(CanAttackKey.SelectedKeyName, Distance < AttackDistance);
	}
	else
	{
		BB->ClearValue(TargetActorKey.SelectedKeyName);
		BB->SetValueAsBool(CanAttackKey.SelectedKeyName, false);
	}

	// ----------------------
	// LOS + FOV 체크
	// ----------------------
	// 1) 기본 LOS(가려짐 없음)
	bool bHasLOS = AICon->LineOfSightTo(Player);

	// 2) FOV(시야각) 체크
	//    - 수평면 기준 체크 옵션(bUsePlanarFOV) 지원
	FVector EyeLoc = SelfPawn->GetPawnViewLocation();      // 없으면 SelfLoc 써도 됩니다.
	FVector ToTarget = TargetLoc - EyeLoc;
	FVector Forward = SelfPawn->GetActorForwardVector();

	if (bUsePlanarFOV)
	{
		ToTarget.Z = 0.f;
		Forward.Z = 0.f;
	}

	const float LenToTarget = ToTarget.Size();
	const float LenForward = Forward.Size();
	bool bInFOV = false;

	if (LenToTarget > KINDA_SMALL_NUMBER && LenForward > KINDA_SMALL_NUMBER)
	{
		ToTarget.Normalize();
		Forward.Normalize();

		const float HalfFOVCos = FMath::Cos(FMath::DegreesToRadians(FOVDegrees * 0.5f));
		const float Dot = FVector::DotProduct(Forward, ToTarget);
		bInFOV = (Dot >= HalfFOVCos);
	}

	// 3) 최종 시야 가시성 = LOS ∧ FOV
	const bool bVisible = bHasLOS && bInFOV;

	if (HasLineOfSightKey.SelectedKeyType)
	{
		BB->SetValueAsBool(HasLineOfSightKey.SelectedKeyName, bVisible);
	}

	// 4) 마지막 위치 갱신(보일 때만)
	if (bVisible && LastKnownLocationKey.SelectedKeyType)
	{
		BB->SetValueAsVector(LastKnownLocationKey.SelectedKeyName, TargetLoc);
	}
}
