#include "BT/BTService_LockPlayerTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"

UBTService_LockPlayerTarget::UBTService_LockPlayerTarget()
{
    NodeName = TEXT("Lock Player As Target (No Perception)");
    Interval = 0.2f;
    RandomDeviation = 0.f;
    bNotifyTick = true;
}

void UBTService_LockPlayerTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
    Super::InitializeFromAsset(Asset);
    UBlackboardData* BB = GetBlackboardAsset();
    if (!BB) return;

    InBattleKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_LockPlayerTarget, InBattleKey));
    InBattleKey.ResolveSelectedKey(*BB);

    TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_LockPlayerTarget, TargetActorKey), AActor::StaticClass());
    TargetActorKey.ResolveSelectedKey(*BB);
}


AActor* UBTService_LockPlayerTarget::PickTarget(UWorld* World, APawn* Self) const
{
    if (!World) return nullptr;

    // 1) �÷��̾� �� �ľ�
    TArray<AActor*> Players;
    UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), Players);

    // ����: PlayerControlled�� ����
    TArray<AActor*> PlayerCandidates;
    for (AActor* A : Players)
    {
        ACharacter* C = Cast<ACharacter>(A);
        if (!C) continue;
        if (AController* Ctrl = C->GetController())
        {
            if (Ctrl->IsPlayerController())
            {
                PlayerCandidates.Add(C);
            }
        }
    }

    if (PlayerCandidates.Num() == 0)
    {
        // �̱� ���� ���� ���(���忡 PlayerController 0�� ���� ������ ���)
        if (APawn* P0 = UGameplayStatics::GetPlayerPawn(World, 0))
        {
            return P0;
        }
        return nullptr;
    }

    if (!bPickClosestInMP)
    {
        // �׳� ù ��°
        return PlayerCandidates[0];
    }

    // ���� ����� �÷��̾�
    float BestDistSq = TNumericLimits<float>::Max();
    AActor* Best = nullptr;
    const FVector MyLoc = Self ? Self->GetActorLocation() : FVector::ZeroVector;

    for (AActor* Cand : PlayerCandidates)
    {
        const float d2 = FVector::DistSquared(MyLoc, Cand->GetActorLocation());
        if (d2 < BestDistSq)
        {
            BestDistSq = d2;
            Best = Cand;
        }
    }
    return Best;
}

void UBTService_LockPlayerTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return;

    // ���� ���� �ƴϸ� �ƹ��͵� �� ��
    if (InBattleKey.SelectedKeyType && !BB->GetValueAsBool(InBattleKey.SelectedKeyName))
        return;

    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* SelfPawn = AICon ? AICon->GetPawn() : nullptr;
    UWorld* World = SelfPawn ? SelfPawn->GetWorld() : nullptr;
    if (!World) return;

    UObject* CurObj = TargetActorKey.SelectedKeyType ? BB->GetValueAsObject(TargetActorKey.SelectedKeyName) : nullptr;
    AActor* CurTarget = Cast<AActor>(CurObj);

    // �̹� Ÿ���� �ְ�, "�� �� ���ϸ� ���� ����" �ɼ��̸� �״�� �д�.
    if (CurTarget && bLockFirstPickForever) return;

    // Ÿ���� ���ų� ��ȿ�� �ٽ� ��
    if (!CurTarget)
    {
        if (AActor* NewTarget = PickTarget(World, SelfPawn))
        {
            BB->SetValueAsObject(TargetActorKey.SelectedKeyName, NewTarget);
        }
        return;
    }

    // ���� ������ �ƴϸ�, ��Ƽ���� "����� �÷��̾�� ����Ī"�� ���� �� ����
    if (!bLockFirstPickForever && bPickClosestInMP)
    {
        if (AActor* Best = PickTarget(World, SelfPawn))
        {
            if (Best != CurTarget)
            {
                BB->SetValueAsObject(TargetActorKey.SelectedKeyName, Best);
            }
        }
    }
}