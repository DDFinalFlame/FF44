#pragma once
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_LockPlayerTarget.generated.h"

UCLASS()
class MONSTERAIPLUGIN_API UBTService_LockPlayerTarget : public UBTService
{
    GENERATED_BODY()
public:
    UBTService_LockPlayerTarget();

protected:
    virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
    virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector InBattleKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetActorKey;

    // 멀티플레이일 때 타깃 선정 정책
    UPROPERTY(EditAnywhere, Category = "Target")
    bool bPickClosestInMP = true;

    // 최초 한번만 픽하고 절대 바꾸지 않기(멀티에서 "첫 어그로 고정" 원할 때)
    UPROPERTY(EditAnywhere, Category = "Target")
    bool bLockFirstPickForever = false;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector BossStateKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector PrevBossStateKey;

    // === BossState: Hit 처리 ===
    UPROPERTY(EditAnywhere, Category = "BossState")
    FName HitStateName = TEXT("Hit");

    UPROPERTY(EditAnywhere, Category = "BossState")
    bool bTreatZeroAsUnset = true;

    uint8 HitStateValue = 0;
    bool  bHitResolved = false;
    uint8 CachedLastState = 0;

private:
    AActor* PickTarget(UWorld* World, APawn* Self) const; // 싱글/멀티 대응
    bool ResolveHitFromBlackboardEnum();
};