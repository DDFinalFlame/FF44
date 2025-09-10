#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EnemyWeaponControl.generated.h"

enum class EWeaponType : uint8;

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
	virtual void ActivateWeaponCollision(EWeaponType WeaponType) = 0;
	virtual void DeactivateWeaponCollision(EWeaponType WeaponType) = 0;
	virtual bool IsAttackSuccessful() = 0;
};
