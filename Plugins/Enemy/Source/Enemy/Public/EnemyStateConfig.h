// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "EnemyDefine.h"
#include "EnemyStateConfig.generated.h"

/**
 *
 */
USTRUCT(BlueprintType)
struct FEnemyStateConfig
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EAIBehavior, bool> InterruptibleMap;

	bool CheckIsInterruptible(EAIBehavior TargetBehavior)
	{
		if (const bool* FoundValue = InterruptibleMap.Find(TargetBehavior))
		{
			return *FoundValue;
		}
		else
		{
			return false;
		}
	}
};
