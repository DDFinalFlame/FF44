// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EQS_Context_AttackTarget.generated.h"

/**
 * 
 */
UCLASS()
class ENEMY_API UEQS_Context_AttackTarget : public UEnvQueryContext
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, Category = "EQS")
	FName TargetKeyName = "F_Target";

public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;

};
