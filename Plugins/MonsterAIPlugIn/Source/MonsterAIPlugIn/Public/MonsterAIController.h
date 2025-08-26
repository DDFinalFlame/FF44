// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "GenericTeamAgentInterface.h"
#include "MonsterAIController.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERAIPLUGIN_API AMonsterAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AMonsterAIController();

    // 팀 반환
    virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }
    // 태도(우호/적대) 판단
    virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

protected:
	virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
	
    // Perception 콜백
    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // 필요시 BB 업데이트 헬퍼
    void UpdateBlackboardKeys();

protected:
    // === BehaviorTree/BB ===
    UPROPERTY(EditAnywhere, Category = "AI")
    class UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(EditAnywhere, Category = "AI")
    class UBlackboardData* BBAsset;

    UPROPERTY()
    class UBlackboardComponent* BlackboardComponent;

    // === Perception ===
    UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
    class UAIPerceptionComponent* PerceptionComp;

    UPROPERTY()
    class UAISenseConfig_Sight* SightConfig;

    UPROPERTY()
    class UAISenseConfig_Hearing* HearingConfig;

    // 시야/청각 기본값(에디터에서 수정 가능)
    UPROPERTY(EditAnywhere, Category = "AI|Perception")
    float SightRadius = 1500.f;

    UPROPERTY(EditAnywhere, Category = "AI|Perception")
    float LoseSightRadius = 1800.f;

    UPROPERTY(EditAnywhere, Category = "AI|Perception")
    float PeripheralVisionAngle = 90.f; // 총 FOV 각도

    UPROPERTY(EditAnywhere, Category = "AI|Perception")
    float HearingRange = 1200.f;

    UPROPERTY(EditAnywhere, Category = "AI|Perception")
    float LoSHearingRange = 1500.f;

    // 블랙보드 키 이름(프로젝트 키와 맞추세요)
    UPROPERTY(EditAnywhere, Category = "AI|Blackboard")
    FName BBKey_TargetActor = TEXT("TargetActor");

    UPROPERTY(EditAnywhere, Category = "AI|Blackboard")
    FName BBKey_HasLineOfSight = TEXT("HasLineOfSight");

    UPROPERTY(EditAnywhere, Category = "AI|Blackboard")
    FName BBKey_LastKnownLocation = TEXT("LastKnownLocation");

    FGenericTeamId TeamId = FGenericTeamId(1); // 몬스터 팀=1
};
