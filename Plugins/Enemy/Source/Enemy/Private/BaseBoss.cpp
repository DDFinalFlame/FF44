// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseBoss.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SplineComponent.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "Weapon/EnemyBaseWeapon.h"
#include "Weapon/EnemyFXWeapon.h"

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
		bMovingUp = true;
	}
	if (bMovingUp && !bMovingHand)
	{
		// TO-DO : �� ���� ���� �����̱� - tick �ؾ��ϴµ� 
		// TRY1 : ���� z �� ��������

 		USkeletalMeshComponent* mesh = GetMesh();
		FVector BoneWorldPos = mesh->GetSocketLocation(HandSocketName);

		FVector TargetLocation = Weapon->GetActorLocation();
		TargetLocation.Z = BoneWorldPos.Z;

		Weapon->SetActorLocation(TargetLocation);
	}
	if (bMovingHand && SplineComponent)
	{
		// 1) ���ö��� ���� / �Ÿ� ������Ʈ
		float SplineLength = SplineComponent->GetSplineLength();
		DistanceAlongSpline += MoveSpeed * DeltaSeconds * MoveSpeedMultiplier;
		DistanceAlongSpline = FMath::Clamp(DistanceAlongSpline, 0.f, SplineLength);

		bool bReachedEnd = (DistanceAlongSpline >= SplineLength);
		if (bReachedEnd)
		{
			//bMovingHand = false;
			// Particle ����

			return;
		}
		// 2) ���� ��ġ
		FVector CurrentLocation = SplineComponent->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);

		// 3) look-ahead �Ÿ� ��� (�ӵ��� ����ϰ� �ϰų� ����������)
		float LookAheadDistance = MoveSpeed * LookAheadSeconds; // �Ǵ� ������: 30.f
		float NextDistance = FMath::Clamp(DistanceAlongSpline + LookAheadDistance, 0.f, SplineLength);
		FVector TargetLocation = SplineComponent->GetLocationAtDistanceAlongSpline(NextDistance, ESplineCoordinateSpace::World);

		// 4) ��ǥ ȸ�� ��� (���� �ٶ󺸰�)
		FRotator TargetRot = (TargetLocation - CurrentLocation).Rotation(); // X ���� '��'���� ����ϴ� ȸ��

		// --- ���� Weapon�� �� �������� X�� �ƴ϶�� ���� �ʿ� (��: Forward�� +Y�̸� Yaw ���� ��)
		 FRotator ModelForwardAdjust = FRotator(0.f, 90.f, 0.f);
		 TargetRot += ModelForwardAdjust;

		// 5) �ε巯�� ȸ�� (���ϸ�)
		FRotator CurrentRot = Weapon->GetActorRotation();
		FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaSeconds, RotationInterpSpeed);

		// 6) ��ġ/ȸ�� ����
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

void ABaseBoss::SendEventToTarget(FGameplayTag EventTag)
{
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIController->GetBlackboardComponent())
		{
			if (AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(FName("F_Target"))))
			{
				FGameplayEventData EventData;

				EventData.EventTag = EventTag;
				EventData.Instigator = GetOwner()->GetOwner();
				EventData.Target = TargetActor;

				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
					TargetActor,
					EventData.EventTag,
					EventData
				);
			}
		}
	}
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
	// ���ö��� ��ġ ����
	// ��ġ�� ��� �޾ƿðŴ�
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

void ABaseBoss::ToggleCollision(bool bStartEvade)
{
	// Evade ���� ? �ݸ��� false
	// �ٵ� �̷� �ʿ� ���� Hit �� ������ �Ǵ� �� �ƴѰ�? 
	//SetActorEnableCollision(!bStartEvade);
}

void ABaseBoss::ToggleDissolve(bool bStartEvade)
{
	// Evade ���� ? Hidden true
	SetActorHiddenInGame(bStartEvade);
}
