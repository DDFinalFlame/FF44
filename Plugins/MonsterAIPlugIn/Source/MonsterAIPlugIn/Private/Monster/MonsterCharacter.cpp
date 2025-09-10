
#include "Monster/MonsterCharacter.h"
#include "AbilitySystemComponent.h"
#include "MonsterAttributeSet.h"
#include "GAS/GE_MonsterDefaultStat.h"
#include "GAS/GA_MonsterAttack.h"
#include "GameFramework/PlayerState.h" // OnRep_PlayerState 위해 필요
#include "AI/MonsterAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"  
#include "BehaviorTree/BehaviorTree.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "GAS/GA_HitReact.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "MonsterTags.h"
#include "Weapon/MonsterBaseWeapon.h"


AMonsterCharacter::AMonsterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UMonsterAttributeSet>(TEXT("AttributeSet"));

	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetReceivesDecals(false);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	AIControllerClass = AMonsterAIController::StaticClass();

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		//RVO 켜기
		Move->bUseRVOAvoidance = true;

		//RVO가 실제로 동작하도록 기본 파라미터(프로젝트에 맞게 조정)
		Move->AvoidanceConsiderationRadius = 180.f; // 서로를 인지하는 반경
		Move->AvoidanceWeight = 0.5f;               // 회피 가중치(0~1)

		// 이동 회전 설정은 기존 그대로 사용 (필요 시만 조정)
		Move->bOrientRotationToMovement = true;
		Move->bUseControllerDesiredRotation = false;
		Move->RotationRate = FRotator(0.f, 540.f, 0.f);
	}
}


void AMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (USkeletalMeshComponent* Sk = GetMesh())
	{
		MeshInitRelLoc = Sk->GetRelativeLocation();
		MeshInitRelRot = Sk->GetRelativeRotation();
		MeshInitRelScale = Sk->GetRelativeScale3D();
	}


	// Definition 로드
	if (!MonsterDefinition.IsValid())		MonsterDefinition.LoadSynchronous();
	UMonsterDefinition* Def = MonsterDefinition.Get();
	if (!Def) { UE_LOG(LogTemp, Warning, TEXT("MonsterDefinition not set.")); return; }

	// Mesh / AnimBP 적용
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (!Def->SkeletalMesh.IsValid()) Def->SkeletalMesh.LoadSynchronous();
		MeshComp->SetSkeletalMesh(Def->SkeletalMesh.Get());

		if (!Def->AnimBlueprint.IsValid()) Def->AnimBlueprint.LoadSynchronous();
		MeshComp->SetAnimInstanceClass(Def->AnimBlueprint.Get());
	}

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

	if (HasAuthority() && AbilitySystemComponent)
	{
		int32 AbilityIndex = 0;
		for (int32 i = 0; i < Def->AbilitiesToGrant.Num(); ++i)
		{
			TSubclassOf<UGameplayAbility> GA = Def->AbilitiesToGrant[i];
			if (GA)
				AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(GA, 1, AbilityIndex++));
		}
	}
	
	// DT에서 스탯 읽어 SetByCaller GE 적용
	if (MonsterStatTable && Def->StatRowName.IsValid())
	{
		if (FMonsterStatRow* Row = MonsterStatTable->FindRow<FMonsterStatRow>(Def->StatRowName, TEXT("MonsterInit")))
		{
			ApplyInitStats(*Row, Def->InitStatGE_SetByCaller);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Row %s not found in MonsterStatTable."), *Def->StatRowName.ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MonsterStatTable or StatRowName not set."));
	}

	// AI(BT/BB) 연결
	if (AMonsterAIController* AIC = Cast<AMonsterAIController>(GetController()))
	{
		// 소프트 레퍼런스 동기 로드
		if (!Def->BehaviorTree.IsValid())    Def->BehaviorTree.LoadSynchronous();
		if (!Def->BlackboardAsset.IsValid()) Def->BlackboardAsset.LoadSynchronous();

		UBlackboardComponent* BBComp = nullptr; // out 파라미터를 받을 lvalue 준비
		const bool bBBInited = AIC->UseBlackboard(Def->BlackboardAsset.Get(), BBComp);
		if (bBBInited && BBComp)
		{
			if (MonsterStatTable && Def->StatRowName.IsValid())
			{
				if (FMonsterStatRow* Row2 = MonsterStatTable->FindRow<FMonsterStatRow>(Def->StatRowName, TEXT("MonsterInit")))
				{
					BBComp->SetValueAsFloat(TEXT("DetectDistance"), Row2->DetectDistance);
					BBComp->SetValueAsFloat(TEXT("AttackDistance"), Row2->AttackDistance);
				}
			}

			SetMonsterState(StartState);

			AIC->RunBehaviorTree(Def->BehaviorTree.Get());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UseBlackboard failed or BBComp is null."));
		}
	}


	if (AbilitySystemComponent)
	{
		const FGameplayTag DeadTag = MonsterTags::State_Dead;
		AbilitySystemComponent
			->RegisterGameplayTagEvent(DeadTag, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &AMonsterCharacter::OnDeadTagChanged);
	}


	SetupAttackCollision();
}


// Called every frame
void AMonsterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

//	//시야 전용 디버깅
//#if WITH_EDITOR
//
//	if (!bDebugLOS) return;
//
//	AAIController* AICon = Cast<AAIController>(GetController());
//	if (!AICon) return;
//
//	UBlackboardComponent* BB = AICon->GetBlackboardComponent();
//	if (!BB) return;
//
//	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
//	if (!Player) return;
//
//	const bool bVisible = BB->GetValueAsBool(TEXT("HasLineOfSight"));
//	const FVector Eye = GetPawnViewLocation();
//	const FVector TargetLoc = Player->GetActorLocation();
//
//	DrawDebugLine(GetWorld(), Eye, TargetLoc, bVisible ? FColor::Green : FColor::Red, false, 0.f, 0, 2.f);
//	DrawDebugString(GetWorld(), Eye + FVector(0, 0, 50),
//		bVisible ? TEXT("LOS+FOV: TRUE") : TEXT("LOS+FOV: FALSE"),
//		nullptr, bVisible ? FColor::Green : FColor::Red, 0.f, true);
//
//#endif
}

UAbilitySystemComponent* AMonsterCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AMonsterCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

void AMonsterCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

void AMonsterCharacter::Attack()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->TryActivateAbilityByClass(UGA_MonsterAttack::StaticClass());
	}
}


void AMonsterCharacter::SetMonsterState(EMonsterState NewState)
{
	if (CurrentState == EMonsterState::Dead) return; // 죽으면 상태 못 바꿈
	if (CurrentState == NewState) return;

	CurrentState = NewState;

	// 상태 변경 시 로깅
	UE_LOG(LogTemp, Log, TEXT("[Monster] State changed to %d"), (uint8)NewState);

	if (NewState == EMonsterState::AmbushReady)
	{
		BeginAmbushBoost();
	}
	else
	{
		EndAmbushBoost();
	}

	SyncStateToBlackboard();
}


void AMonsterCharacter::ApplyInitStats(const FMonsterStatRow& Row, TSubclassOf<class UGameplayEffect> InitGE)
{
	if (!AbilitySystemComponent || !InitGE) return;

	FGameplayEffectContextHandle Ctx = AbilitySystemComponent->MakeEffectContext();
	FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(InitGE, 1.f, Ctx);
	if (!Spec.IsValid()) return;

	const FGameplayTag Tag_MaxHealth = MonsterTags::Data_MaxHealth;
	const FGameplayTag Tag_Health = MonsterTags::Data_Health;
	const FGameplayTag Tag_Attack = MonsterTags::Data_AttackPower;
	const FGameplayTag Tag_Move = MonsterTags::Data_MoveSpeed;
	const FGameplayTag Tag_Defense = MonsterTags::Data_Defense;

	Spec.Data->SetSetByCallerMagnitude(Tag_MaxHealth, Row.MaxHealth);
	Spec.Data->SetSetByCallerMagnitude(Tag_Health, Row.MaxHealth);
	Spec.Data->SetSetByCallerMagnitude(Tag_Attack, Row.AttackPower);
	Spec.Data->SetSetByCallerMagnitude(Tag_Move, Row.MoveSpeed);
	Spec.Data->SetSetByCallerMagnitude(Tag_Defense, Row.Defense);

	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

	/*if (GEngine && AttributeSet)
	{
		FString Msg = FString::Printf(TEXT("HP=%.1f / Max=%.1f  ATK=%.1f Defense=%.1f Move=%.1f "),
			AttributeSet->GetHealth(),
			AttributeSet->GetMaxHealth(),
			AttributeSet->GetAttackPower(),
			AttributeSet->GetDefense(),
			AttributeSet->GetMoveSpeed());
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, Msg);
	}*/

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = Row.MoveSpeed;
		//기습 전용 기본 데이터 저장.
		DefaultWalkSpeed = Row.MoveSpeed;
	}
}

void AMonsterCharacter::SyncStateToBlackboard()
{
	if (AMonsterAIController* AIC = Cast<AMonsterAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			// BB에 정수(State)로 밀어넣기 (BB에 Int 키 "State" 준비)
			BB->SetValueAsInt(TEXT("State"), static_cast<int32>(CurrentState));

			// (원하시면 호환용 Bool도 유지 가능)
			BB->SetValueAsBool(TEXT("IsAmbush"), CurrentState == EMonsterState::AmbushReady);
		}
	}
}

void AMonsterCharacter::SetTargetActor(FName KeyName, AActor* NewTarget)
{
	AAIController* AICon = Cast<AAIController>(GetController());
	if (!AICon) { SpawnDefaultController(); AICon = Cast<AAIController>(GetController()); }
	if (!AICon) return;

	if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
	{
		BB->SetValueAsObject(KeyName, NewTarget);
	}
}

void AMonsterCharacter::FinishAmbush()
{
	// BT 태스크나 AnimNotify에서 호출: 기습 끝 → 일반 루트로
	SetMonsterState(EMonsterState::CombatReady);
}


// 몬스터 기습 헬퍼함수.
void AMonsterCharacter::BeginAmbushBoost()
{
	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (Move)
	{
		float base = DefaultWalkSpeed > 0.f ? DefaultWalkSpeed : Move->MaxWalkSpeed;
		Move->MaxWalkSpeed = base * AmbushSpeedRate;
	}
}

void AMonsterCharacter::EndAmbushBoost()
{
	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (Move)
	{
		float base = DefaultWalkSpeed > 0.f ? DefaultWalkSpeed : 600.f; // 안전값
		Move->MaxWalkSpeed = base;
	}
}


void AMonsterCharacter::OnDeadTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	const bool bDead = (NewCount > 0);

	// 내부 상태 갱신
	if (bDead) CurrentState = EMonsterState::Dead;

	// BB에 반영
	if (AMonsterAIController* AIC = Cast<AMonsterAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			BB->SetValueAsBool(TEXT("IsDead"), bDead);
			BB->SetValueAsInt(TEXT("State"), static_cast<int32>(CurrentState));
		}
		// 죽으면 즉시 AI/이동 정지
		if (bDead)
		{
			if (AIC->BrainComponent) AIC->BrainComponent->StopLogic(TEXT("Dead"));
		}
	}

	if (bDead)
	{
		if (UCharacterMovementComponent* Mv = GetCharacterMovement())
		{
			Mv->StopMovementImmediately();
			Mv->DisableMovement();
		}
		if (UCapsuleComponent* Cap = GetCapsuleComponent())
		{
			Cap->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}


void AMonsterCharacter::ActivateAttackHitbox(bool bEnable)
{
	// 파생에서 특정 박스만 On/Off 하려면 오버라이드하세요.
	for (UPrimitiveComponent* Comp : AttackHitboxes)
	{
		if (!Comp) continue;
		Comp->SetGenerateOverlapEvents(bEnable);
	}
}

void AMonsterCharacter::BeginAttackWindow()
{
	bAttackActive = true;
	HitActorsThisSwing.Reset();
	TArray<AMonsterBaseWeapon*> Equips;
	GetEquippedWeapons(Equips);

	if (Equips.Num() > 0)
	{
		// 무기 기반 판정
		if (HasAuthority())
		{
			for (AMonsterBaseWeapon* W : Equips)
				if (W) W->BeginAttackWindow();
		}
	}
	else
	{
		// 레거시: 캐릭터 자체 히트박스
		ActivateAttackHitbox(true);
	}
}

void AMonsterCharacter::EndAttackWindow()
{
	TArray<AMonsterBaseWeapon*> Equips;
	GetEquippedWeapons(Equips);

	if (Equips.Num() > 0)
	{
		if (HasAuthority())
		{
			for (AMonsterBaseWeapon* W : Equips)
				if (W) W->EndAttackWindow();
		}
	}
	else
	{
		ActivateAttackHitbox(false);
	}

	bAttackActive = false;
	HitActorsThisSwing.Reset();
}

void AMonsterCharacter::RegisterWeapon(AMonsterBaseWeapon* NewWeapon)
{
	if (!NewWeapon) return;
	Weapons.AddUnique(NewWeapon);

	// 하위 호환: 기존 코드가 Weapon만 참조해도 동작하도록 첫 등록 시 세팅
	if (!Weapon) Weapon = NewWeapon;
}

void AMonsterCharacter::GetEquippedWeapons(TArray<AMonsterBaseWeapon*>& OutWeapons) const
{
	// 다무기 우선
	for (AMonsterBaseWeapon* W : Weapons)
		if (W) OutWeapons.Add(W);

	// 그래도 없으면(구 몬스터) 단일 무기 포인터 사용
	if (OutWeapons.Num() == 0 && Weapon)
		OutWeapons.Add(Weapon);
}


void AMonsterCharacter::PushAttackCollision()
{
	AttackCollisionDepth++;
	ApplyCollisionProfile();
}

void AMonsterCharacter::PopAttackCollision()
{
	AttackCollisionDepth = FMath::Max(0, AttackCollisionDepth - 1);
	ApplyCollisionProfile();
}

void AMonsterCharacter::ApplyCollisionProfile()
{
	const FName Profile = (AttackCollisionDepth > 0) ? AttackingProfile : DefaultProfile;
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionProfileName(Profile,  true);
	}
}

float AMonsterCharacter::GetAttackPower_Implementation() const
{
	// 1) ASC에 등록된 AttributeSet에서 우선 조회
	if (AbilitySystemComponent)
	{
		const UMonsterAttributeSet* FromASC = AbilitySystemComponent->GetSet<UMonsterAttributeSet>();
		if (FromASC)
		{
			const float AP = FromASC->GetAttackPower();
			return FMath::IsFinite(AP) ? AP : 0.f;
		}
	}

	// 2) 폴백: 멤버 보관용 AttributeSet에서 조회
	if (AttributeSet)
	{
		const float AP = AttributeSet->GetAttackPower();
		return FMath::IsFinite(AP) ? AP : 0.f;
	}

	return 0.f;
}


void AMonsterCharacter::EnterRagdollState()
{
	USkeletalMeshComponent* sk = GetMesh();
	if (sk)
	{
		sk->SetCollisionProfileName(TEXT("Ragdoll"));
		sk->SetAllBodiesSimulatePhysics(true);
		sk->WakeAllRigidBodies();
		sk->bPauseAnims = true;
		sk->SetAllBodiesPhysicsBlendWeight(1.f);
	}

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
	}

	AAIController* aic = Cast<AAIController>(GetController());
	if (aic && aic->BrainComponent)
	{
		aic->BrainComponent->StopLogic(TEXT("Ragdoll"));
	}

	SetMonsterState(EMonsterState::Ragdoll);
}

