#include "BT/BTService_LockPlayerTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h" 
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

    BossStateKey.SelectedKeyName = TEXT("BossState");     
    PrevBossStateKey.SelectedKeyName = TEXT("PrevBossState"); 
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

    
    BossStateKey.ResolveSelectedKey(*BB);
    PrevBossStateKey.ResolveSelectedKey(*BB);

    bHitResolved = ResolveHitFromBlackboardEnum();
}

void UBTService_LockPlayerTarget::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) // [�߰�]
{
    Super::OnBecomeRelevant(OwnerComp, NodeMemory);

    if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
    {
        if (BossStateKey.SelectedKeyType)
        {
            CachedLastState = BB->GetValueAsEnum(BossStateKey.SelectedKeyName);
        }
    }
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

    {
        // �ʿ�� ���ؼ�(��Ÿ�ӿ� BB ���� ���ɼ� ���)
        if (!bHitResolved)
        {
            bHitResolved = ResolveHitFromBlackboardEnum();
        }

        if (BossStateKey.SelectedKeyType && PrevBossStateKey.SelectedKeyType)
        {
            const uint8 Current = BB->GetValueAsEnum(BossStateKey.SelectedKeyName);

            if (Current != CachedLastState)
            {
                const bool bIsHit = (bHitResolved && Current == HitStateValue);

                // �� ���°� Hit�� �ƴϸ� Prev = ���� ���·� ���
                if (!bIsHit)
                {
                    if (!(bTreatZeroAsUnset && CachedLastState == 0))
                    {
                        BB->SetValueAsEnum(PrevBossStateKey.SelectedKeyName, CachedLastState);
                    }
                }

                CachedLastState = Current;
            }
        }
    }

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

bool UBTService_LockPlayerTarget::ResolveHitFromBlackboardEnum() // [�߰�]
{
    if (!BossStateKey.SelectedKeyType) return false;

    const UBlackboardKeyType_Enum* EnumKey =
        Cast<UBlackboardKeyType_Enum>(BossStateKey.SelectedKeyType);
    if (!EnumKey || !EnumKey->EnumType) return false;

    UEnum* StateEnum = EnumKey->EnumType;

    // 1) �̸����� ���� ��ȸ
    int64 Value = StateEnum->GetValueByName(HitStateName);
    if (Value == INDEX_NONE)
    {
        // 2) "EBossState::Hit" ���±��� ����(��ҹ��� ����)
        const int32 Count = StateEnum->NumEnums();
        for (int32 i = 0; i < Count; ++i)
        {
            const FString FullName = StateEnum->GetNameStringByIndex(i);
            const FString RightToken = FullName.Contains(TEXT("::"))
                ? FullName.RightChop(FullName.Find(TEXT("::")) + 2)
                : FullName;

            if (RightToken.Equals(HitStateName.ToString(), ESearchCase::IgnoreCase))
            {
                Value = StateEnum->GetValueByIndex(i);
                break;
            }
        }
    }

    if (Value == INDEX_NONE) return false;

    HitStateValue = static_cast<uint8>(Value);
    return true;
}