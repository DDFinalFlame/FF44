// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_UpdateCrowdState.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERAIPLUGIN_API UBTService_UpdateCrowdState : public UBTService_BlackboardBase
{
    GENERATED_BODY()
public:
    UBTService_UpdateCrowdState();

protected:
    // ---- Crowd ���� ----
    UPROPERTY(EditAnywhere, Category = "Crowd")
    float CheckRadius = 1000.f;

    UPROPERTY(EditAnywhere, Category = "Crowd")
    int32 ThresholdCount = 3; // N �̻��̸� CombatReady

    // ---- Proximity(����) ���� ----
    UPROPERTY(EditAnywhere, Category = "Proximity")
    bool bUseProximity = true;

    UPROPERTY(EditAnywhere, Category = "Proximity", meta = (EditCondition = "bUseProximity"))
    float ProximityRadius = 1200.f; // �� �Ÿ� ���̸� Patrol

    UPROPERTY(EditAnywhere, Category = "Crowd")
    float ProximityExitPadding = 100.f;     // ���� ��Ż ����(= exit�� enter+padding)

    UPROPERTY(EditAnywhere, Category = "Crowd")
    float StateChangeCooldown = 0.4f;       // ���� ���� �ּ� ����(��)

    // ---- Blackboard Keys ----
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetActorKey;   // �÷��̾�

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector MonsterStateKey;  // Enum Ű (EMonsterState)

    // (����) �Ÿ� Ű�� �о� ��� ���� ���̱�
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector DistanceToTargetKey;

    UPROPERTY(EditAnywhere, Category = "BB")
    FBlackboardKeySelector LastStateChangeTimeKey;  // Float (TimeSeconds ���)

    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, Category = "State Mapping")
    bool bSyncCharacterState = true;

private:
    int32 CountNearbyAllies(AActor* Center, float Radius, APawn* SelfPawn) const;

    int32 CountAttackingAllies(AActor* Center, float Radius, APawn* SelfPawn) const;

};