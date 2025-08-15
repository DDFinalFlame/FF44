
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
//static FGameplayTag TAG_Event_Hit() { return FGameplayTag::RequestGameplayTag(TEXT("Event.Hit")); }
// Sets default values
AMonsterCharacter::AMonsterCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UMonsterAttributeSet>(TEXT("AttributeSet"));

	AIControllerClass = AMonsterAIController::StaticClass();

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	//피격전용 트리거 설정.
	HitTestTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("HitTestTrigger"));
	HitTestTrigger->SetupAttachment(GetRootComponent());
	HitTestTrigger->InitSphereRadius(120.f);               
	HitTestTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HitTestTrigger->SetCollisionObjectType(ECC_WorldDynamic);
	HitTestTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	HitTestTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // 플레이어만 Overlap
	HitTestTrigger->SetGenerateOverlapEvents(true);
}

// Called when the game starts or when spawned
void AMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();

	

	// Definition 로드
	if (!MonsterDefinition.IsValid())
		MonsterDefinition.LoadSynchronous();
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

		//// 데이터 에셋에 안 넣고 코드로 바로 붙이고 싶으면:
		//if (!AbilitySystemComponent->FindAbilitySpecFromClass(UGA_HitReact::StaticClass()))
		//{
		//	AbilitySystemComponent->GiveAbility(
		//		FGameplayAbilitySpec(UGA_HitReact::StaticClass(), 1, AbilityIndex++));
		//}
	}

	//// Ability 부여
	//if (AbilitySystemComponent)
	//{
	//	int32 AbilityIndex = 0;
	//	for (int32 i = 0; i < Def->AbilitiesToGrant.Num(); ++i)
	//	{
	//		TSubclassOf<UGameplayAbility> GA = Def->AbilitiesToGrant[i];
	//		if (GA)
	//		{
	//			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(GA, 1, AbilityIndex++));
	//		}
	//	}
	//}

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
			// BT 실행


			// 필요 시 초기 BB 값 주입
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


	// 고정형 GE 적용 로직
	//if (HasAuthority() && AbilitySystemComponent)
	//{
	//	// C++로 만든 GameplayEffect를 생성
	//	UGE_MonsterDefaultStat* DefaultStatEffect = NewObject<UGE_MonsterDefaultStat>();
	//	if (DefaultStatEffect)
	//	{
	//		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	//		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultStatEffect->GetClass(), 1.f, Context);

	//		if (SpecHandle.IsValid())
	//		{
	//			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	//		}
	//	}
	//	AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(UGA_MonsterAttack::StaticClass(), 1, 0));

	//}

	if (HitTestTrigger)
	{
		FTimerHandle Tmp;
		GetWorld()->GetTimerManager().SetTimer(Tmp, [this]()
			{
				if (HitTestTrigger)
				{
					HitTestTrigger->OnComponentBeginOverlap.AddDynamic(this, &AMonsterCharacter::OnHitTestBegin);
				}
			}, 0.02f, false);
	}
}


// Called every frame
void AMonsterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


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

	const FGameplayTag Tag_Health = FGameplayTag::RequestGameplayTag(FName("Data.Health"));
	const FGameplayTag Tag_Attack = FGameplayTag::RequestGameplayTag(FName("Data.AttackPower"));
	const FGameplayTag Tag_Move = FGameplayTag::RequestGameplayTag(FName("Data.MoveSpeed"));

	Spec.Data->SetSetByCallerMagnitude(Tag_Health, Row.MaxHealth);
	Spec.Data->SetSetByCallerMagnitude(Tag_Attack, Row.AttackPower);
	Spec.Data->SetSetByCallerMagnitude(Tag_Move, Row.MoveSpeed);

	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

	if (GEngine && AttributeSet)
	{
		FString Msg = FString::Printf(TEXT("HP=%.1f  ATK=%.1f"),
			AttributeSet->GetHealth(),
			AttributeSet->GetAttackPower());
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
}