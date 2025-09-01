#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BossAttack.generated.h"

class ABaseEnemy;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UBossAttack : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class ENEMY_API IBossAttack
{
	GENERATED_BODY()

public:
	// For Summon Logic
	virtual bool IsReadyToSummon() const = 0;
	virtual int GetSummonNum() const = 0;
	virtual void RequestSummonLocation() = 0;
	virtual const TArray<FVector>& GetSummonLocation() const = 0;

	// For Recall Logic
	virtual void AddSpawnedEnemy(TWeakObjectPtr<ABaseEnemy> Enemy) = 0;
	virtual void DeleteSpawnedEnemy(TWeakObjectPtr<ABaseEnemy> Enemy) = 0;
	virtual TArray<TWeakObjectPtr<ABaseEnemy>> GetGhostList() = 0;
};
