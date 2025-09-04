// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseBoss.h"

#include "Components/SplineComponent.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "Weapon/EnemyBaseWeapon.h"

ABaseBoss::ABaseBoss()
{
	//SplineComponent = CreateDefaultSubobject<USplineComponent>("HandPath");
}

void ABaseBoss::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//// Hand
	//if (bMovingHand)
	//{
	//	if (!SplineComponent) return;

	//	// 스플라인 길이 가져오기
	//	float SplineLength = SplineComponent->GetSplineLength();

	//	// DistanceAlongSpline 증가
	//	DistanceAlongSpline += MoveSpeed * DeltaSeconds;
	//	if (DistanceAlongSpline > SplineLength)
	//		DistanceAlongSpline = SplineLength; // 끝에 도달하면 멈춤

	//	// 현재 스플라인 위치 계산
	//	FVector NewLocation = SplineComponent->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);

	//	// 액터 위치 변경
	//	Weapon->SetActorLocation(NewLocation);
	//}

	// Temp
	Weapon->SetActorLocation(GetActorLocation() + GetActorForwardVector() * 300.0f);
}

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

FVector ABaseBoss::GetBossLocation()
{
	return GetActorLocation();
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

void ABaseBoss::ActivateWeaponCollision()
{
	FVector FowardV = GetActorForwardVector();

	Weapon->SetActorLocation(GetActorLocation() + FowardV * 1000.0f);
	Weapon->SetActorRotation(GetActorRotation());

	Super::ActivateWeaponCollision();

	bMovingHand = true;
}

void ABaseBoss::DeactivateWeaponCollision()
{
	Super::DeactivateWeaponCollision();

	bMovingHand = false;
}
