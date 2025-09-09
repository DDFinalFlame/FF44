// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AT_Evade.h"

#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/EnvQueryTypes.h"

UAT_Evade::UAT_Evade(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	bTickingTask = true;
}

UAT_Evade* UAT_Evade::AbilityTick(UGameplayAbility* OwningAbility)
{
	UAT_Evade* MyTask = NewAbilityTask<UAT_Evade>(OwningAbility);
	return MyTask;
}

void UAT_Evade::Activate()
{
	Super::Activate();

	FEnvQueryRequest QueryRequest(EndPosQueryTemplate, this);

	QueryRequest.Execute(EEnvQueryRunMode::SingleResult, this, &UAT_Evade::OnQueryFinished);

}

void UAT_Evade::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (CachedLocations.IsEmpty()) { return; }
}

void UAT_Evade::OnQueryFinished(TSharedPtr<FEnvQueryResult> Result)
{
	if (Result->IsSuccessful())
	{
		Result->GetAllAsLocations(CachedLocations);
	}
	else
	{
		// 종료 시켜버려
		OnFailed.Broadcast();
	}
}
