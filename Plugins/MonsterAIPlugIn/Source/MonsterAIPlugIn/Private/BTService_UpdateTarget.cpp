#include "BTService_UpdateTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTService_UpdateTarget::UBTService_UpdateTarget()
{
	NodeName = TEXT("Update Distance & CanAttack (Read Target from Perception)");

	// 서비스 호출 간격
	Interval = 0.2f;
	RandomDeviation = 0.0f;

	bNotifyBecomeRelevant = false;
	bNotifyTick = true;
}

void UBTService_UpdateTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	UBlackboardData* BBAsset = GetBlackboardAsset();
	if (!BBAsset) return;

	// Perception이 넣어주는 키들: 읽기 전용
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateTarget, TargetActorKey), AActor::StaticClass());
	TargetActorKey.ResolveSelectedKey(*BBAsset);

	// 서비스가 세팅하는 키들
	CanAttackKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateTarget, CanAttackKey));
	CanAttackKey.ResolveSelectedKey(*BBAsset);

	DistanceToTargetKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateTarget, DistanceToTargetKey));
	DistanceToTargetKey.ResolveSelectedKey(*BBAsset);

	// (선택) BB에서 공격 사거리 읽기
	AttackDistanceKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateTarget, AttackDistanceKey));
	AttackDistanceKey.ResolveSelectedKey(*BBAsset);

	// (선택) Perception이 넣는 시야 여부를 읽어 CanAttack 조건에 포함
	HasLineOfSightKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateTarget, HasLineOfSightKey));
	HasLineOfSightKey.ResolveSelectedKey(*BBAsset);
}

void UBTService_UpdateTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return;

	APawn* SelfPawn = AICon->GetPawn();
	if (!SelfPawn) return;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;

	// Perception이 세팅한 TargetActor만 읽는다 (여기서 세팅/클리어하지 않음)
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));

	// 타깃 없으면 초기화 후 종료
	if (!Target)
	{
		if (DistanceToTargetKey.SelectedKeyType)
			BB->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, FLT_MAX);

		if (CanAttackKey.SelectedKeyType)
			BB->SetValueAsBool(CanAttackKey.SelectedKeyName, false);

		return;
	}

	// 거리 계산 및 기록
	const float Distance = FVector::Dist(Target->GetActorLocation(), SelfPawn->GetActorLocation());
	if (DistanceToTargetKey.SelectedKeyType)
		BB->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, Distance);

	// 공격 사거리 결정: BB > 기본값
	float AttackRange = DefaultAttackDistance;
	if (AttackDistanceKey.SelectedKeyType)
	{
		const float FromBB = BB->GetValueAsFloat(AttackDistanceKey.SelectedKeyName);
		if (FromBB > 0.f) AttackRange = FromBB;
	}

	// (선택) 시야 필요 조건 적용
	bool bLOSOK = true;
	if (bRequireLOSForCanAttack && HasLineOfSightKey.SelectedKeyType)
	{
		bLOSOK = BB->GetValueAsBool(HasLineOfSightKey.SelectedKeyName);
	}

	const bool bCanAttack = (Distance <= AttackRange) && bLOSOK;

	if (CanAttackKey.SelectedKeyType)
		BB->SetValueAsBool(CanAttackKey.SelectedKeyName, bCanAttack);
}