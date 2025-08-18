
#include "MonsterCharacter.h"
#include "AbilitySystemComponent.h"
#include "MonsterAttributeSet.h"
#include "GE_MonsterDefaultStat.h"
#include "GA_MonsterAttack.h"
#include "GameFramework/PlayerState.h" // OnRep_PlayerState 위해 필요
#include "MonsterAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"  
#include "BehaviorTree/BehaviorTree.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "GA_HitReact.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"

AMonsterCharacter::AMonsterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UMonsterAttributeSet>(TEXT("AttributeSet"));

	AIControllerClass = AMonsterAIController::StaticClass();

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}


void AMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();

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
		const FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(TEXT("State.Dead"));
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

	if (HasAuthority())
	{
		UpdateTransition_PatrolToCombatReady();
	}

#if WITH_EDITOR

	if (!bDebugLOS) return;

	AAIController* AICon = Cast<AAIController>(GetController());
	if (!AICon) return;

	UBlackboardComponent* BB = AICon->GetBlackboardComponent();
	if (!BB) return;

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!Player) return;

	const bool bVisible = BB->GetValueAsBool(TEXT("HasLineOfSight"));
	const FVector Eye = GetPawnViewLocation();
	const FVector TargetLoc = Player->GetActorLocation();

	DrawDebugLine(GetWorld(), Eye, TargetLoc, bVisible ? FColor::Green : FColor::Red, false, 0.f, 0, 2.f);
	DrawDebugString(GetWorld(), Eye + FVector(0, 0, 50),
		bVisible ? TEXT("LOS+FOV: TRUE") : TEXT("LOS+FOV: FALSE"),
		nullptr, bVisible ? FColor::Green : FColor::Red, 0.f, true);

#endif
}


void AMonsterCharacter::UpdateTransition_PatrolToCombatReady()
{
	if (CurrentState != EMonsterState::Patrol) return;

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!Player) return;

	float Detect = 0.f;

	AMonsterAIController* AIC = Cast<AMonsterAIController>(GetController());
	if (AIC)
	{
		UBlackboardComponent* BB = AIC->GetBlackboardComponent();
		if (BB)
		{
			Detect = BB->GetValueAsFloat(TEXT("DetectDistance"));
		}
	}

	// 폴백: 캐시 → 기본값
	if (Detect <= 0.f && DetectDistanceCache > 0.f)
	{
		Detect = DetectDistanceCache;
	}
	if (Detect <= 0.f)
	{
		Detect = FallbackDetectDistance;
	}

	const float Dist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
	if (Dist > Detect) return;

	SetMonsterState(EMonsterState::CombatReady);

	if (AIC)
	{
		UBlackboardComponent* BB = AIC->GetBlackboardComponent();
		if (BB)
		{
			BB->SetValueAsObject(TEXT("TargetActor"), Player);
		}
	}
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

	const FGameplayTag Tag_MaxHealth = FGameplayTag::RequestGameplayTag(FName("Data.MaxHealth"));
	const FGameplayTag Tag_Health = FGameplayTag::RequestGameplayTag(FName("Data.Health"));
	const FGameplayTag Tag_Attack = FGameplayTag::RequestGameplayTag(FName("Data.AttackPower"));
	const FGameplayTag Tag_Move = FGameplayTag::RequestGameplayTag(FName("Data.MoveSpeed"));

	Spec.Data->SetSetByCallerMagnitude(Tag_MaxHealth, Row.MaxHealth);
	Spec.Data->SetSetByCallerMagnitude(Tag_Health, Row.MaxHealth);
	Spec.Data->SetSetByCallerMagnitude(Tag_Attack, Row.AttackPower);
	Spec.Data->SetSetByCallerMagnitude(Tag_Move, Row.MoveSpeed);

	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

	if (GEngine && AttributeSet)
	{
		FString Msg = FString::Printf(TEXT("HP=%.1f / Max=%.1f  ATK=%.1f  Move=%.1f"),
			AttributeSet->GetHealth(),
			AttributeSet->GetMaxHealth(),
			AttributeSet->GetAttackPower(),
			AttributeSet->GetMoveSpeed());
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, Msg);
	}

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


void AMonsterCharacter::OnHitTestBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this) return;

	// 플레이어만 인정 (필요하면 팀/태그 체크로 바꾸세요)
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (OtherActor != Player) return;

	// 연속 트리거 방지
	const double Now = GetWorld()->GetTimeSeconds();
	if (Now - LastHitTime < HitCooldown) return;
	LastHitTime = Now;

	TriggerHitReact(Player);
}

void AMonsterCharacter::TriggerHitReact(AActor* InstigatorActor)
{
	// 1) Event.Hit 전송 → GA_HitReact 발동
	FGameplayEventData Payload;
	Payload.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Hit"));
	Payload.Instigator = InstigatorActor ? InstigatorActor : this;
	Payload.Target = this;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, FGameplayTag::RequestGameplayTag(TEXT("Event.Hit")), Payload);

	if (AbilitySystemComponent)
	{
		bool bOk = AbilitySystemComponent->TryActivateAbilityByClass(UGA_HitReact::StaticClass());
		UE_LOG(LogTemp, Warning, TEXT("TryActivateAbilityByClass HitReact: %s"), bOk ? TEXT("OK") : TEXT("FAIL"));
	}

	// 2) (선택) 데미지 적용
	if (TestDamageGE && AbilitySystemComponent)
	{
		FGameplayEffectContextHandle Ctx = AbilitySystemComponent->MakeEffectContext();
		Ctx.AddInstigator(InstigatorActor ? InstigatorActor : this, GetController());
		AbilitySystemComponent->ApplyGameplayEffectToSelf(
			TestDamageGE->GetDefaultObject<UGameplayEffect>(), 1.f, Ctx);
	}

	// 상태를 Hit으로 전환
	SetMonsterState(EMonsterState::Hit);
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


void AMonsterCharacter::RegisterHitbox(UPrimitiveComponent* Comp)
{
	if (!Comp) return;
	AttackHitboxes.Add(Comp);

	Comp->SetGenerateOverlapEvents(false); // 기본은 Off (창/검이 켜질 때만 On)
	Comp->OnComponentBeginOverlap.AddDynamic(this, &AMonsterCharacter::OnAttackHitboxBeginOverlap);

	// 충돌 설정 예시(원하시는 프로필로 교체 가능)
	Comp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Comp->SetCollisionObjectType(ECC_WorldDynamic);
	Comp->SetCollisionResponseToAllChannels(ECR_Ignore);
	Comp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // 플레이어만 맞추려면 여기에
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
	ActivateAttackHitbox(true);
}

void AMonsterCharacter::EndAttackWindow()
{
	ActivateAttackHitbox(false);
	bAttackActive = false;
	HitActorsThisSwing.Reset();
}

void AMonsterCharacter::OnAttackHitboxBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!bAttackActive) return;
	if (!OtherActor || OtherActor == this) return;

	// 팀/플레이어 필터링 필요 시 여기에서 태그/인터페이스 체크
	if (HitActorsThisSwing.Contains(OtherActor)) return; // 중복 타격 방지

	HitActorsThisSwing.Add(OtherActor);
	ApplyMeleeHitTo(OtherActor, SweepResult);
}

void AMonsterCharacter::ApplyMeleeHitTo(AActor* Victim, const FHitResult& Hit)
{
	// 1) Hit 이벤트 트리거 → GA_HitReact 발동(현재 구조 활용)
	{
		FGameplayEventData Payload;
		Payload.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Hit"));
		Payload.Instigator = this;
		Payload.Target = Victim;
		// 필요 시 ByCaller 데미지 전달도 가능 (Payload에 Magnitude 등 확장)
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			Victim, Payload.EventTag, Payload);
	}

	// 2) 데미지 적용 (예: TestDamageGE 사용 or 데미지 전용 GE)
	if (AbilitySystemComponent)
	{
		// ByCaller로 데미지 수치 전달을 추천합니다.
		// 지금은 간단하게 TestDamageGE 적용 + GetAttackDamage 반영 예시:
		if (TestDamageGE)
		{
			FGameplayEffectContextHandle Ctx = AbilitySystemComponent->MakeEffectContext();
			Ctx.AddInstigator(this, GetController());

			FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(
				TestDamageGE, 1.f, Ctx);

			if (Spec.IsValid())
			{
				// ByCaller 태그 예시: "Data.Damage" 가 있다고 가정
				const FGameplayTag Tag_Damage = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
				Spec.Data->SetSetByCallerMagnitude(Tag_Damage, GetAttackDamage());

				// 타겟의 ASC에 적용
				UAbilitySystemComponent* TargetASC =
					UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Victim);
				if (TargetASC)
				{
					TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
				}
			}
		}
	}

	// 3) 후처리 훅(파생에서 이펙트/사운드 등)
	OnAttackHit(Victim);
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