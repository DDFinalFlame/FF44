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
	if (CurrentBehavior == EAIBehavior::Die) { return; }

	Super::Tick(DeltaSeconds);

	AEnemyBaseWeapon** FoundValue = WeaponMap.Find(EWeaponType::FXHand);
	if (!*FoundValue) { return; }
 	AEnemyBaseWeapon* Weapon = *FoundValue;
	if (!Weapon) { return; }

	// Hand
	if (Weapon->IsAttackSuccessful())
	{
		bMovingHand = false;
		bMovingUp = true;
	}
	if (bMovingUp && !bMovingHand)
	{
		// TO-DO : 손 위로 조금 움직이기 - tick 해야하는데 
		// TRY1 : 소켓 z 값 가져오기

 		USkeletalMeshComponent* mesh = GetMesh();
		FVector BoneWorldPos = mesh->GetSocketLocation(HandSocketName);

		FVector TargetLocation = Weapon->GetActorLocation();
		TargetLocation.Z = BoneWorldPos.Z;

		Weapon->SetActorLocation(TargetLocation);
	}
	if (bMovingHand && SplineComponent)
	{
		// 1) 스플라인 길이 / 거리 업데이트
		float SplineLength = SplineComponent->GetSplineLength();
		DistanceAlongSpline += MoveSpeed * DeltaSeconds * MoveSpeedMultiplier;
		DistanceAlongSpline = FMath::Clamp(DistanceAlongSpline, 0.f, SplineLength);

		bool bReachedEnd = (DistanceAlongSpline >= SplineLength);
		if (bReachedEnd)
		{
			//bMovingHand = false;
			// Particle 끄기

			return;
		}
		// 2) 현재 위치
		FVector CurrentLocation = SplineComponent->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);

		// 3) look-ahead 거리 계산 (속도에 비례하게 하거나 고정값으로)
		float LookAheadDistance = MoveSpeed * LookAheadSeconds; // 또는 고정값: 30.f
		float NextDistance = FMath::Clamp(DistanceAlongSpline + LookAheadDistance, 0.f, SplineLength);
		FVector TargetLocation = SplineComponent->GetLocationAtDistanceAlongSpline(NextDistance, ESplineCoordinateSpace::World);

		// 4) 목표 회전 계산 (앞을 바라보게)
		FRotator TargetRot = (TargetLocation - CurrentLocation).Rotation(); // X 축을 '앞'으로 사용하는 회전

		// --- 만약 Weapon의 모델 전방축이 X가 아니라면 보정 필요 (예: Forward가 +Y이면 Yaw 보정 등)
		 FRotator ModelForwardAdjust = FRotator(0.f, 90.f, 0.f);
		 TargetRot += ModelForwardAdjust;

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

void ABaseBoss::ActivateWeaponCollision(EWeaponType WeaponType)
{
	// 스플라인 위치 설정
	// 위치를 어떻게 받아올거니
	if (WeaponType == EWeaponType::FXHand)
	{
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

	}

	Super::ActivateWeaponCollision(WeaponType);

	bMovingHand = true;
}

void ABaseBoss::DeactivateWeaponCollision(EWeaponType WeaponType)
{
	Super::DeactivateWeaponCollision(WeaponType);

	bMovingHand = false;
	DistanceAlongSpline = 0.0f;
}

void ABaseBoss::ToggleCollision(bool bStartEvade)
{
	// Evade 시작 ? 콜리전 false
	// 근데 이럴 필요 없이 Hit 만 막으면 되는 거 아닌가? 
	//SetActorEnableCollision(!bStartEvade);
}

void ABaseBoss::ToggleDissolve(bool bStartEvade)
{
	// Evade 시작 ? Hidden true
	//SetActorHiddenInGame(bStartEvade);
	// Mesh 만 끄자
	// Evade 시작 ? Visible false
	GetMesh()->SetVisibility(!bStartEvade);
}

void ABaseBoss::SetPhase(float currentHP, float maxHp)
{
	// Valid 체크
	AAIController* AIController = Cast<AAIController>(GetController());
	if (!AIController) { return; }
	UBlackboardComponent* BB = AIController->GetBlackboardComponent();
	if (!BB) { return; }

	int32 CurrentPhase = BB->GetValueAsInt(PhaseKey);

	int32 NewPhase;
	// 페이즈 계산
	if (currentHP <= maxHp * 0.3f)      // 30% 이하 : Phase 3
	{
		NewPhase = 3;
	}
	else if (currentHP <= maxHp * 0.6f) // 60% 이하 : Phase 2
	{
		NewPhase = 2;
	}
	else
	{
		NewPhase = 1;
	}

	if (NewPhase != CurrentPhase)
	{
		CurrentPhase = NewPhase;

		// 이미 개막 패턴 실행 여부 확인
		bool bAlreadyTriggered = false;
		if (NewPhase == 1) bAlreadyTriggered = bPhase1Triggered;
		if (NewPhase == 2) bAlreadyTriggered = bPhase2Triggered;
		if (NewPhase == 3) bAlreadyTriggered = bPhase3Triggered;

		if (!bAlreadyTriggered)
		{
			BB->SetValueAsInt(PhaseKey, CurrentPhase);
			BB->SetValueAsBool(bOpeningPatternDoneKey, false);

			UE_LOG(LogTemp, Log, TEXT("Boss entered Phase %d"), CurrentPhase);

			// 해당 Phase를 "이미 실행함" 으로 표시
			if (NewPhase == 1) bPhase1Triggered = true;
			if (NewPhase == 2) bPhase2Triggered = true;
			if (NewPhase == 3) bPhase3Triggered = true;
		}

	}
}
