#include "BTService_UpdateTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

UBTService_UpdateTarget::UBTService_UpdateTarget()
{
	NodeName = TEXT("Update Target & CanAttack");

	// ���� ȣ�� ���� (�����Ϳ����� ���� ����)
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

	// �Ÿ� ����
	const FVector SelfLoc = SelfPawn->GetActorLocation();
	const FVector TargetLoc = Player->GetActorLocation();
	const float   Distance = FVector::Dist(TargetLoc, SelfLoc);

	if (DistanceToTargetKey.SelectedKeyType)
	{
		BB->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, Distance);
	}

	// Ÿ��/���� ���� ����
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
	// LOS + FOV üũ
	// ----------------------
	// 1) �⺻ LOS(������ ����)
	bool bHasLOS = AICon->LineOfSightTo(Player);

	// 2) FOV(�þ߰�) üũ
	//    - ����� ���� üũ �ɼ�(bUsePlanarFOV) ����
	FVector EyeLoc = SelfPawn->GetPawnViewLocation();      // ������ SelfLoc �ᵵ �˴ϴ�.
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

	// 3) ���� �þ� ���ü� = LOS �� FOV
	const bool bVisible = bHasLOS && bInFOV;

	if (HasLineOfSightKey.SelectedKeyType)
	{
		BB->SetValueAsBool(HasLineOfSightKey.SelectedKeyName, bVisible);
	}

	// 4) ������ ��ġ ����(���� ����)
	if (bVisible && LastKnownLocationKey.SelectedKeyType)
	{
		BB->SetValueAsVector(LastKnownLocationKey.SelectedKeyName, TargetLoc);
	}
}
