#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_EnemyHit.generated.h"

/**
 * 
 */
UCLASS()
class ENEMY_API UGE_EnemyHit : public UGameplayEffect
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag HitGrantTag;

public:
	UGE_EnemyHit();
};
