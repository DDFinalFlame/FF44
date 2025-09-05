// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "MyEnvQueryContext_Target.generated.h"

UCLASS(EditInlineNew)
class MONSTERAIPLUGIN_API UMyEnvQueryContext_Target : public UEnvQueryContext
{
	GENERATED_BODY()

public:
    // BB���� Ÿ���� �о�� Ű �̸� (Object Ÿ�� ����)
    UPROPERTY(EditDefaultsOnly, Category = "EQS|Context")
    FName TargetKeyName = TEXT("TargetActor"); 

    virtual void ProvideContext(FEnvQueryInstance& QueryInstance,
        FEnvQueryContextData& ContextData) const override;
};
