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
    // ---- Crowd 기준 ----
    UPROPERTY(EditAnywhere, Category = "Crowd")
    float CheckRadius = 1000.f;

    UPROPERTY(EditAnywhere, Category = "Crowd")
    int32 ThresholdCount = 3; // N 이상이면 CombatReady

    // ---- Proximity(근접) 기준 ----
    UPROPERTY(EditAnywhere, Category = "Proximity")
    bool bUseProximity = true;

    UPROPERTY(EditAnywhere, Category = "Proximity", meta = (EditCondition = "bUseProximity"))
    float ProximityRadius = 1200.f; // 이 거리 밖이면 Patrol

    UPROPERTY(EditAnywhere, Category = "Crowd")
    float ProximityExitPadding = 100.f;     // 전투 이탈 여유(= exit는 enter+padding)

    UPROPERTY(EditAnywhere, Category = "Crowd")
    float StateChangeCooldown = 0.4f;       // 상태 변경 최소 간격(초)

    // ---- Blackboard Keys ----
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetActorKey;   // 플레이어

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector MonsterStateKey;  // Enum 키 (EMonsterState)

    // (선택) 거리 키를 읽어 비싼 연산 줄이기
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector DistanceToTargetKey;

    UPROPERTY(EditAnywhere, Category = "BB")
    FBlackboardKeySelector LastStateChangeTimeKey;  // Float (TimeSeconds 기록)

    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, Category = "State Mapping")
    bool bSyncCharacterState = true;

private:
    int32 CountNearbyAllies(AActor* Center, float Radius, APawn* SelfPawn) const;

    int32 CountAttackingAllies(AActor* Center, float Radius, APawn* SelfPawn) const;

};