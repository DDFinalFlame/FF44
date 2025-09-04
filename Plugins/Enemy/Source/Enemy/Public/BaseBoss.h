// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseEnemy.h"
#include "Interfaces/BossAttack.h"
#include "BaseBoss.generated.h"

class USplineComponent;
struct FEnvQueryResult;
class UEnvQuery;
/**
 *
 */
UCLASS()
class ENEMY_API ABaseBoss : public ABaseEnemy, public IBossAttack
{
	GENERATED_BODY()


protected:
	// Summon ��ġ ã�� EQS
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | EQS")
	UEnvQuery* SummonQueryTemplate;

	// EQS ���
	bool bSummonLocationsReady = false;
	TArray<FVector> CachedSummonLocations;

protected:
	// Summon �� Actor��
	TArray<TWeakObjectPtr<ABaseEnemy>> SpawnedGhosts;

protected:
	// Grab ���
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USplineComponent* SplineComponent;

	// Spline ��� �̵� ����
	bool bMovingHand = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceAlongSpline = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LookAheadSeconds = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotationInterpSpeed = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveSpeed = 200.f; // ���ö��� �̵� �ӵ�

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveSpeedMultiplier = 3.f; // ���ö��� �̵� �ӵ�

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

public:
	// EQS Finish ���ε��� ��������Ʈ �Լ�
	void OnSummonQueryFinished(TSharedPtr<FEnvQueryResult> Result);

public:
	// ��� ��Ʈ�� �߰��� Activate/Deactivate
	virtual void ActivateWeaponCollision() override;
	virtual void DeactivateWeaponCollision() override;
};
