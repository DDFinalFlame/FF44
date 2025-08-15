
#include "MonsterCharacter.h"
#include "AbilitySystemComponent.h"
#include "MonsterAttributeSet.h"
#include "GE_MonsterDefaultStat.h"
#include "GA_MonsterAttack.h"
#include "GameFramework/PlayerState.h" // OnRep_PlayerState ���� �ʿ�
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

	//�ǰ����� Ʈ���� ����.
	HitTestTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("HitTestTrigger"));
	HitTestTrigger->SetupAttachment(GetRootComponent());
	HitTestTrigger->InitSphereRadius(120.f);               
	HitTestTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HitTestTrigger->SetCollisionObjectType(ECC_WorldDynamic);
	HitTestTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	HitTestTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // �÷��̾ Overlap
	HitTestTrigger->SetGenerateOverlapEvents(true);
}

// Called when the game starts or when spawned
void AMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();

	

	// Definition �ε�
	if (!MonsterDefinition.IsValid())
		MonsterDefinition.LoadSynchronous();
	UMonsterDefinition* Def = MonsterDefinition.Get();
	if (!Def) { UE_LOG(LogTemp, Warning, TEXT("MonsterDefinition not set.")); return; }

	// Mesh / AnimBP ����
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

		//// ������ ���¿� �� �ְ� �ڵ�� �ٷ� ���̰� ������:
		//if (!AbilitySystemComponent->FindAbilitySpecFromClass(UGA_HitReact::StaticClass()))
		//{
		//	AbilitySystemComponent->GiveAbility(
		//		FGameplayAbilitySpec(UGA_HitReact::StaticClass(), 1, AbilityIndex++));
		//}
	}

	//// Ability �ο�
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

	// DT���� ���� �о� SetByCaller GE ����
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

	// AI(BT/BB) ����
	if (AMonsterAIController* AIC = Cast<AMonsterAIController>(GetController()))
	{
		// ����Ʈ ���۷��� ���� �ε�
		if (!Def->BehaviorTree.IsValid())    Def->BehaviorTree.LoadSynchronous();
		if (!Def->BlackboardAsset.IsValid()) Def->BlackboardAsset.LoadSynchronous();

		UBlackboardComponent* BBComp = nullptr; // out �Ķ���͸� ���� lvalue �غ�
		const bool bBBInited = AIC->UseBlackboard(Def->BlackboardAsset.Get(), BBComp);
		if (bBBInited && BBComp)
		{
			// BT ����


			// �ʿ� �� �ʱ� BB �� ����
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


	// ������ GE ���� ����
	//if (HasAuthority() && AbilitySystemComponent)
	//{
	//	// C++�� ���� GameplayEffect�� ����
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
	if (CurrentState == EMonsterState::Dead) return; // ������ ���� �� �ٲ�
	if (CurrentState == NewState) return;

	CurrentState = NewState;

	// ���� ���� �� �α�
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
		//��� ���� �⺻ ������ ����.
		DefaultWalkSpeed = Row.MoveSpeed;
	}
}

void AMonsterCharacter::SyncStateToBlackboard()
{
	if (AMonsterAIController* AIC = Cast<AMonsterAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			// BB�� ����(State)�� �о�ֱ� (BB�� Int Ű "State" �غ�)
			BB->SetValueAsInt(TEXT("State"), static_cast<int32>(CurrentState));

			// (���Ͻø� ȣȯ�� Bool�� ���� ����)
			BB->SetValueAsBool(TEXT("IsAmbush"), CurrentState == EMonsterState::AmbushReady);
		}
	}
}

void AMonsterCharacter::FinishAmbush()
{
	// BT �½�ũ�� AnimNotify���� ȣ��: ��� �� �� �Ϲ� ��Ʈ��
	SetMonsterState(EMonsterState::CombatReady);
}


// ���� ��� �����Լ�.
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
		float base = DefaultWalkSpeed > 0.f ? DefaultWalkSpeed : 600.f; // ������
		Move->MaxWalkSpeed = base;
	}
}


void AMonsterCharacter::OnHitTestBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this) return;

	// �÷��̾ ���� (�ʿ��ϸ� ��/�±� üũ�� �ٲټ���)
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (OtherActor != Player) return;

	// ���� Ʈ���� ����
	const double Now = GetWorld()->GetTimeSeconds();
	if (Now - LastHitTime < HitCooldown) return;
	LastHitTime = Now;

	TriggerHitReact(Player);
}

void AMonsterCharacter::TriggerHitReact(AActor* InstigatorActor)
{
	// 1) Event.Hit ���� �� GA_HitReact �ߵ�
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

	// 2) (����) ������ ����
	if (TestDamageGE && AbilitySystemComponent)
	{
		FGameplayEffectContextHandle Ctx = AbilitySystemComponent->MakeEffectContext();
		Ctx.AddInstigator(InstigatorActor ? InstigatorActor : this, GetController());
		AbilitySystemComponent->ApplyGameplayEffectToSelf(
			TestDamageGE->GetDefaultObject<UGameplayEffect>(), 1.f, Ctx);
	}
}