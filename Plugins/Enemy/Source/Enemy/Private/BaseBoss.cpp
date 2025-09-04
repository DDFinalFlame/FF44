// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseBoss.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SplineComponent.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "Weapon/EnemyBaseWeapon.h"

ABaseBoss::ABaseBoss()
{
	SplineComponent = CreateDefaultSubobject<USplineComponent>("HandPath");

	SplineComponent->SetupAttachment(GetMesh());
}

void ABaseBoss::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Hand
	if (Weapon->IsAttackSuccessful())
	{
		bMovingHand = false;

		// TO-DO :
		// Target의 위치에 멈춰야함
		// 혹은 Target을 손에 붙여주던가
	}
	if (bMovingHand && SplineComponent)
	{
		// 1) 스플라인 길이 / 거리 업데이트
		float SplineLength = SplineComponent->GetSplineLength();
		DistanceAlongSpline += MoveSpeed * DeltaSeconds * MoveSpeedMultiplier;
		DistanceAlongSpline = FMath::Clamp(DistanceAlongSpline, 0.f, SplineLength);

		// 2) 현재 위치
		FVector CurrentLocation = SplineComponent->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);

		// 3) look-ahead 거리 계산 (속도에 비례하게 하거나 고정값으로)
		float LookAheadDistance = MoveSpeed * LookAheadSeconds; // 또는 고정값: 30.f
		float NextDistance = FMath::Clamp(DistanceAlongSpline + LookAheadDistance, 0.f, SplineLength);
		FVector TargetLocation = SplineComponent->GetLocationAtDistanceAlongSpline(NextDistance, ESplineCoordinateSpace::World);

		// 4) 목표 회전 계산 (앞을 바라보게)
		FRotator TargetRot = (TargetLocation - CurrentLocation).Rotation(); // X 축을 '앞'으로 사용하는 회전

		// --- 만약 Weapon의 모델 전방축이 X가 아니라면 보정 필요 (예: Forward가 +Y이면 Yaw 보정 등)
		// FRotator ModelForwardAdjust = FRotator(0.f, 90.f, 0.f);
		// TargetRot += ModelForwardAdjust;

		// 5) 부드러운 회전 (원하면)
		FRotator CurrentRot = Weapon->GetActorRotation();
		FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaSeconds, RotationInterpSpeed);

		// 6) 위치/회전 적용
		Weapon->SetActorLocationAndRotation(CurrentLocation, NewRot);
	}
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
	// 스플라인 위치 설정
	// 위치를 어떻게 받아올거니
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIController->GetBlackboardComponent())
		{
			if (AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(FName("F_Target"))))
			{
				FVector TargetLocation = TargetActor->GetActorLocation();
				SplineComponent->SetWorldLocation(TargetLocation);
			}

		}
	}

	Super::ActivateWeaponCollision();

	bMovingHand = true;
}

void ABaseBoss::DeactivateWeaponCollision()
{
	Super::DeactivateWeaponCollision();

	bMovingHand = false;
	DistanceAlongSpline = 0.0f;
}
