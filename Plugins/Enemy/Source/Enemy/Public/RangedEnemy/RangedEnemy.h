// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseEnemy.h"
#include "Interfaces/RangeAttack.h"
#include "RangedEnemy.generated.h"

/**
 * 
 */
UCLASS()
class ENEMY_API ARangedEnemy : public ABaseEnemy, public IRangeAttack
{
	GENERATED_BODY()

protected:
public:
	ARangedEnemy();
// RangeAttack Interface
public:
	virtual FVector GetMuzzleLocation() override;
	virtual FVector GetMuzzleDirection() override;
};
