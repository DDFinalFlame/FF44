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

    // ��Ƽ�÷����� �� Ÿ�� ���� ��å
    UPROPERTY(EditAnywhere, Category = "Target")
    bool bPickClosestInMP = true;

    // ���� �ѹ��� ���ϰ� ���� �ٲ��� �ʱ�(��Ƽ���� "ù ��׷� ����" ���� ��)
    UPROPERTY(EditAnywhere, Category = "Target")
    bool bLockFirstPickForever = false;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector BossStateKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector PrevBossStateKey;

    // === BossState: Hit ó�� ===
    UPROPERTY(EditAnywhere, Category = "BossState")
    FName HitStateName = TEXT("Hit");

    UPROPERTY(EditAnywhere, Category = "BossState")
    bool bTreatZeroAsUnset = true;

    uint8 HitStateValue = 0;
    bool  bHitResolved = false;
    uint8 CachedLastState = 0;

private:
    AActor* PickTarget(UWorld* World, APawn* Self) const; // �̱�/��Ƽ ����
    bool ResolveHitFromBlackboardEnum();
};