#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RangeAttack.generated.h"

class AProjectileBase;

UINTERFACE(MinimalAPI)
class URangeAttack : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ENEMY_API IRangeAttack
{
	GENERATED_BODY()

public:
	virtual FVector GetMuzzleLocation() = 0;
	virtual FVector GetMuzzleDirection() = 0;
};
