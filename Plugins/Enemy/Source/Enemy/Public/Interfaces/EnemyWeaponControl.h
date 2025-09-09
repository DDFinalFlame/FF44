#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EnemyWeaponControl.generated.h"

UINTERFACE(MinimalAPI)
class UEnemyWeaponControl : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ENEMY_API IEnemyWeaponControl
{
	GENERATED_BODY()

public:
	virtual void ActivateWeaponCollision() = 0;
	virtual void DeactivateWeaponCollision() = 0;
	virtual bool IsAttackSuccessful() = 0;
};
