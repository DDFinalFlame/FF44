// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EnemyEvade.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UEnemyEvade : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ENEMY_API IEnemyEvade
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void ToggleCollision(bool bStartEvade) = 0;
	virtual void ToggleDissolve(bool bStartEvade) = 0;

};
