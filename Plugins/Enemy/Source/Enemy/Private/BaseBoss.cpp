// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseBoss.h"

#include "EnvironmentQuery/EnvQueryManager.h"

void ABaseBoss::AddSpawnedEnemy(TWeakObjectPtr<ABaseEnemy> Enemy)
{
	SpawnedGhosts.Add(Enemy);
}

void ABaseBoss::DeleteSpawnedEnemy(TWeakObjectPtr<ABaseEnemy> Enemy)
{
	SpawnedGhosts.Remove(Enemy);
}

TArray<TWeakObjectPtr<ABaseEnemy>> ABaseBoss::GetGhostList()
{
	return SpawnedGhosts;
}

int ABaseBoss::GetSummonNum() const
{
	return 5;
}

void ABaseBoss::RequestSummonLocation()
{
	if (!SummonQueryTemplate) { return; }

	bSummonLocationsReady = false;

	FEnvQueryRequest QueryRequest(SummonQueryTemplate, this);

	QueryRequest.Execute(EEnvQueryRunMode::AllMatching, this, &ABaseBoss::OnSummonQueryFinished);
}

const TArray<FVector>& ABaseBoss::GetSummonLocation() const
{
	return CachedSummonLocations;
}

void ABaseBoss::OnSummonQueryFinished(TSharedPtr<FEnvQueryResult> Result)
{
	if (Result->IsSuccessful())
	{
		CachedSummonLocations.Empty();
		Result->GetAllAsLocations(CachedSummonLocations);
		bSummonLocationsReady = true;
	}
}
