#include "BTService_UpdateTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTService_UpdateTarget::UBTService_UpdateTarget()
{
	NodeName = TEXT("Update Distance & CanAttack (Read Target from Perception)");

	// ���� ȣ�� ����
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

	// Perception�� �־��ִ� Ű��: �б� ����
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateTarget, TargetActorKey), AActor::StaticClass());
	TargetActorKey.ResolveSelectedKey(*BBAsset);

	// ���񽺰� �����ϴ� Ű��
	CanAttackKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateTarget, CanAttackKey));
	CanAttackKey.ResolveSelectedKey(*BBAsset);

	DistanceToTargetKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateTarget, DistanceToTargetKey));
	DistanceToTargetKey.ResolveSelectedKey(*BBAsset);

	// (����) BB���� ���� ��Ÿ� �б�
	AttackDistanceKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateTarget, AttackDistanceKey));
	AttackDistanceKey.ResolveSelectedKey(*BBAsset);

	// (����) Perception�� �ִ� �þ� ���θ� �о� CanAttack ���ǿ� ����
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

	// Perception�� ������ TargetActor�� �д´� (���⼭ ����/Ŭ�������� ����)
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));

	// Ÿ�� ������ �ʱ�ȭ �� ����
	if (!Target)
	{
		if (DistanceToTargetKey.SelectedKeyType)
			BB->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, FLT_MAX);

		if (CanAttackKey.SelectedKeyType)
			BB->SetValueAsBool(CanAttackKey.SelectedKeyName, false);

		return;
	}

	// �Ÿ� ��� �� ���
	const float Distance = FVector::Dist(Target->GetActorLocation(), SelfPawn->GetActorLocation());
	if (DistanceToTargetKey.SelectedKeyType)
		BB->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, Distance);

	// ���� ��Ÿ� ����: BB > �⺻��
	float AttackRange = DefaultAttackDistance;
	if (AttackDistanceKey.SelectedKeyType)
	{
		const float FromBB = BB->GetValueAsFloat(AttackDistanceKey.SelectedKeyName);
		if (FromBB > 0.f) AttackRange = FromBB;
	}

	// (����) �þ� �ʿ� ���� ����
	bool bLOSOK = true;
	if (bRequireLOSForCanAttack && HasLineOfSightKey.SelectedKeyType)
	{
		bLOSOK = BB->GetValueAsBool(HasLineOfSightKey.SelectedKeyName);
	}

	const bool bCanAttack = (Distance <= AttackRange) && bLOSOK;

	if (CanAttackKey.SelectedKeyType)
		BB->SetValueAsBool(CanAttackKey.SelectedKeyName, bCanAttack);
}