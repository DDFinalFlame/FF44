// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseEnemy.h"
#include "Interfaces/BossAttack.h"
#include "Interfaces/EnemyEvade.h"
#include "BaseBoss.generated.h"

class USplineComponent;
struct FEnvQueryResult;
class UEnvQuery;
/**
 *
 */
UCLASS()
class ENEMY_API ABaseBoss : public ABaseEnemy, public IBossAttack, public IEnemyEvade
{
	GENERATED_BODY()


protected:
	// Summon 위치 찾을 EQS
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | EQS")
	UEnvQuery* SummonQueryTemplate;

	// EQS 결과
	bool bSummonLocationsReady = false;
	TArray<FVector> CachedSummonLocations;

protected:
	// Summon 된 Actor들
	TArray<TWeakObjectPtr<ABaseEnemy>> SpawnedGhosts;

protected:
	// Grab 경로
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USplineComponent* SplineComponent;

	// Spline 경로 이동 관련
	bool bMovingHand = false;

	// During Choke Animation
	bool bMovingUp = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName HandSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceAlongSpline = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LookAheadSeconds = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotationInterpSpeed = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveSpeed = 200.f; // 스플라인 이동 속도

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveSpeedMultiplier = 3.f; // 스플라인 이동 속도


protected:
	// Phase - state 관련
	UPROPERTY(EditAnywhere)
	FName PhaseKey;

	UPROPERTY(EditAnywhere)
	FName bOpeningPatternDoneKey;

	bool bPhase1Triggered = false;
	bool bPhase2Triggered = false;
	bool bPhase3Triggered = false;


public:
	ABaseBoss();

protected:
	virtual void Tick(float DeltaSeconds) override;
public:
	// BossAttack Interface - Summon
	FORCEINLINE virtual bool IsReadyToSummon() const override
	{
		return (bSummonLocationsReady) ? true : false;
	}
	virtual int GetSummonNum() const override;
	virtual void RequestSummonLocation() override;
	virtual const TArray<FVector>& GetSummonLocation() const override;

	virtual void AddSpawnedEnemy(TWeakObjectPtr<ABaseEnemy> Enemy) override;
	virtual void DeleteSpawnedEnemy(TWeakObjectPtr<ABaseEnemy> Enemy) override;
	virtual TArray<TWeakObjectPtr<ABaseEnemy>> GetGhostList() override;
	virtual FVector GetBossLocation() override;

	virtual void SendEventToTarget(FGameplayTag EventTag);

public:
	// EQS Finish 바인딩할 델리게이트 함수
	void OnSummonQueryFinished(TSharedPtr<FEnvQueryResult> Result);

public:
	// 경로 컨트롤 추가된 Activate/Deactivate
	virtual void ActivateWeaponCollision(EWeaponType WeaponType) override;
	virtual void DeactivateWeaponCollision(EWeaponType WeaponType) override;

public:
	// Evade Interface
	virtual void ToggleCollision(bool bStartEvade) override;
	virtual void ToggleDissolve(bool bStartEvade) override;

public:
	// Phase - State
	void SetPhase(float currentHP, float maxHp);
};
