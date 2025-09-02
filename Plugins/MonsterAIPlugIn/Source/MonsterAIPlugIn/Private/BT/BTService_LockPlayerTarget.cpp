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

void UBTService_LockPlayerTarget::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) // [추가]
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

    // 1) 플레이어 수 파악
    TArray<AActor*> Players;
    UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), Players);

    // 필터: PlayerControlled만 추출
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
        // 싱글 전용 빠른 경로(월드에 PlayerController 0일 수도 있으니 방어)
        if (APawn* P0 = UGameplayStatics::GetPlayerPawn(World, 0))
        {
            return P0;
        }
        return nullptr;
    }

    if (!bPickClosestInMP)
    {
        // 그냥 첫 번째
        return PlayerCandidates[0];
    }

    // 가장 가까운 플레이어
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
        // 필요시 재해석(런타임에 BB 변경 가능성 대비)
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

                // 새 상태가 Hit가 아니면 Prev = 직전 상태로 기록
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

    // 전투 중이 아니면 아무것도 안 함
    if (InBattleKey.SelectedKeyType && !BB->GetValueAsBool(InBattleKey.SelectedKeyName))
        return;

    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* SelfPawn = AICon ? AICon->GetPawn() : nullptr;
    UWorld* World = SelfPawn ? SelfPawn->GetWorld() : nullptr;
    if (!World) return;

    UObject* CurObj = TargetActorKey.SelectedKeyType ? BB->GetValueAsObject(TargetActorKey.SelectedKeyName) : nullptr;
    AActor* CurTarget = Cast<AActor>(CurObj);

    // 이미 타깃이 있고, "한 번 픽하면 영구 고정" 옵션이면 그대로 둔다.
    if (CurTarget && bLockFirstPickForever) return;

    // 타깃이 없거나 무효면 다시 픽
    if (!CurTarget)
    {
        if (AActor* NewTarget = PickTarget(World, SelfPawn))
        {
            BB->SetValueAsObject(TargetActorKey.SelectedKeyName, NewTarget);
        }
        return;
    }

    // 영구 고정이 아니면, 멀티에서 "가까운 플레이어로 스위칭"을 원할 때 갱신
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

bool UBTService_LockPlayerTarget::ResolveHitFromBlackboardEnum() // [추가]
{
    if (!BossStateKey.SelectedKeyType) return false;

    const UBlackboardKeyType_Enum* EnumKey =
        Cast<UBlackboardKeyType_Enum>(BossStateKey.SelectedKeyType);
    if (!EnumKey || !EnumKey->EnumType) return false;

    UEnum* StateEnum = EnumKey->EnumType;

    // 1) 이름으로 직접 조회
    int64 Value = StateEnum->GetValueByName(HitStateName);
    if (Value == INDEX_NONE)
    {
        // 2) "EBossState::Hit" 형태까지 대응(대소문자 무시)
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