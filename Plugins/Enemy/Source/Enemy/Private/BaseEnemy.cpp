#include "BaseEnemy.h"

#include "AIController.h"
#include "EnemyRotationComponent.h"
#include "HitReactionDataAsset.h"
#include "MonsterAttributeSet.h"
#include "Data/MonsterDefinition.h"
#include "Data/MonsterStatRow.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapon/EnemyBaseWeapon.h"

ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create Component
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("AbilitySystemComponent");


	RotationComponent = CreateDefaultSubobject<UEnemyRotationComponent>("RotationComponent");
}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		if (AbilitySystemComponent)
		{
			UMonsterAttributeSet* AttributeSet = NewObject<UMonsterAttributeSet>(this, UMonsterAttributeSet::StaticClass());
			AbilitySystemComponent->AddAttributeSetSubobject(AttributeSet);
		}
		GiveDefaultAbilities();
		InitializeAttributeSet();


	}

	//TO-DO : 위치 ? 
	// Weapon 생성
	for (auto& WeaponClass : WeaponClasses)
	{
		// GetWorld()로 액터 생성
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		AEnemyBaseWeapon* Weapon = GetWorld()->SpawnActor<AEnemyBaseWeapon>(
			WeaponClass.Value,
			GetActorLocation(),
			GetActorRotation(),
			SpawnParams
		);

		if (Weapon)
		{
			Weapon->SetOwner(this);
			Weapon->EquipWeapon();
		}

		WeaponMap.Add(WeaponClass.Key, Weapon);

	}
}

UAbilitySystemComponent* ABaseEnemy::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABaseEnemy::GiveDefaultAbilities()
{
	if (!AbilitySystemComponent) return;

	for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultAbilities)
	{
		if (AbilityClass)
		{
			FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, static_cast<int32>(INDEX_NONE), this));
			Handles.Add(Handle);
		}
	}

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

FGameplayAbilitySpecHandle ABaseEnemy::RequestAbilityByTag(FGameplayTag AbilityTag)
{
	if (!AbilitySystemComponent)
	{
		return FGameplayAbilitySpecHandle();
	}

	for (const FGameplayAbilitySpecHandle& Handle : Handles)
	{
		FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
		if (!Spec || !Spec->Ability) continue;

		// Ability 클래스(CDO)에 달린 Tags 확인
		const UGameplayAbility* AbilityCDO = Spec->Ability;
		if (AbilityCDO->AbilityTags.HasTagExact(AbilityTag))
		{
			// 실행
			if (AbilitySystemComponent->TryActivateAbility(Handle))
			{
				return Handle; // 성공적으로 실행한 Handle 반환
			}
		}
	}

	return FGameplayAbilitySpecHandle();
}

void ABaseEnemy::InitializeAttributeSet()
{
	if (!MonsterDefinition.IsValid())
	{
		MonsterDefinition.LoadSynchronous();
	}

	UMonsterDefinition* Def = MonsterDefinition.Get();

	if (EnemyDataTable && Def->StatRowName.IsValid())
	{
		if (FMonsterStatRow* Row = EnemyDataTable->FindRow<FMonsterStatRow>(Def->StatRowName, TEXT("MonsterInit")))
		{
			ApplyInitStats(*Row, Def->InitStatGE_SetByCaller);
		}
	}
}

void ABaseEnemy::ApplyInitStats(const FMonsterStatRow& Row, TSubclassOf<class UGameplayEffect> InitGE)
{
	if (!AbilitySystemComponent || !InitGE) return;

	FGameplayEffectContextHandle Ctx = AbilitySystemComponent->MakeEffectContext();
	FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(InitGE, 1.f, Ctx);
	if (!Spec.IsValid()) return;

	const FGameplayTag Tag_MaxHealth = FGameplayTag::RequestGameplayTag(FName("Data.MaxHealth"));
	const FGameplayTag Tag_Health = FGameplayTag::RequestGameplayTag(FName("Data.Health"));
	const FGameplayTag Tag_Attack = FGameplayTag::RequestGameplayTag(FName("Data.AttackPower"));
	const FGameplayTag Tag_Move = FGameplayTag::RequestGameplayTag(FName("Data.MoveSpeed"));
	const FGameplayTag Tag_Defense = FGameplayTag::RequestGameplayTag(FName("Data.Defense"));

	Spec.Data->SetSetByCallerMagnitude(Tag_MaxHealth, Row.MaxHealth);
	Spec.Data->SetSetByCallerMagnitude(Tag_Health, Row.MaxHealth);
	Spec.Data->SetSetByCallerMagnitude(Tag_Attack, Row.AttackPower);
	Spec.Data->SetSetByCallerMagnitude(Tag_Move, Row.MoveSpeed);
	Spec.Data->SetSetByCallerMagnitude(Tag_Defense, Row.Defense);

	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

}

void ABaseEnemy::ActivateWeaponCollision(EWeaponType WeaponType)
{
	if (auto FoundValue = WeaponMap.Find(WeaponType))
	{
		(*FoundValue)->ActivateCollision();
	}
}

void ABaseEnemy::DeactivateWeaponCollision(EWeaponType WeaponType)
{
	if (auto FoundValue = WeaponMap.Find(WeaponType))
	{
		(*FoundValue)->DeactivateCollision();
	}

}

bool ABaseEnemy::IsAttackSuccessful()
{
	for (auto& Weapon : WeaponMap)
	{
		if (Weapon.Value->IsAttackSuccessful())
		{
			return true;
		}
	}

	return false;
}

void ABaseEnemy::SetEnemyState(EAIBehavior NewBehavior)
{
	CurrentBehavior = NewBehavior;
}

bool ABaseEnemy::ChangeState(EAIBehavior NewBehavior)
{
	if (IsCurrentBehaviorEnd || IsCurrentStateInterruptible() || NewBehavior == EAIBehavior::Die)
	{
		CurrentBehavior = NewBehavior;

		if (AAIController* MyController = Cast<AAIController>(GetController()))
		{
			if (UBlackboardComponent* BB = MyController->GetBlackboardComponent())
			{
				IsCurrentBehaviorEnd = false;
				BB->SetValueAsEnum(BehaviorKeyName, static_cast<uint8>(CurrentBehavior));
			}
		}

		return true;
	}

	return false;
}

bool ABaseEnemy::IsCurrentStateInterruptible()
{
	return BehaviorConfig.CheckIsInterruptible(CurrentBehavior);
}

void ABaseEnemy::EndCurrentBehavior()
{
	IsCurrentBehaviorEnd = true;
}

void ABaseEnemy::OnDeath()
{
	/* AI Controller 중단 **/
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->GetBrainComponent()->StopLogic(TEXT("Death"));
		AIController->StopMovement();
		AIController->UnPossess();
	}

	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();

	/* 캡슐 비활성화 **/
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	/* Rotation 중지 **/
	RotationComponent->ToggleShouldRotate(false);
}

// 사용하지 않고 있음. Iron Asset의 Ragdoll 문제
void ABaseEnemy::EndDeath()
{
	FTransform MeshTransform = GetMesh()->GetComponentTransform();
	GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	GetMesh()->SetWorldTransform(MeshTransform);

	/* Mesh Ragdoll 처리 **/
	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		MeshComponent->SetCollisionProfileName("Ragdoll");
		//MeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		MeshComponent->SetSimulatePhysics(true);
	}

	/* 서서히 사라지도록 **/
	StartDissolve();

}

void ABaseEnemy::StartDissolve()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp) return;

	int32 MaterialCount = MeshComp->GetNumMaterials();
	TArray<UMaterialInstanceDynamic*> DynMats;

	for (int32 i = 0; i < MaterialCount; ++i)
	{
		UMaterialInstanceDynamic* DynMat = MeshComp->CreateAndSetMaterialInstanceDynamic(i);
		if (DynMat)
		{
			DynMats.Add(DynMat);
		}
	}

	float Duration = 3.0f;
	float StepTime = 0.05f;
	float Step = StepTime / Duration;
	float CurrentAmount = 0.f;

	GetWorldTimerManager().SetTimer(
		DissolveTimerHandle,
		[this, DynMats, Step, CurrentAmount]() mutable
		{
			CurrentAmount += Step;

			for (UMaterialInstanceDynamic* Mat : DynMats)
			{
				if (Mat)
				{
					Mat->SetScalarParameterValue(FName("Dissolve"), CurrentAmount);
				}
			}

			if (CurrentAmount >= 1.0f)
			{
				GetWorldTimerManager().ClearTimer(DissolveTimerHandle);
				Destroy();
			}
		},
		StepTime,
		true
	);
}

UAnimMontage* ABaseEnemy::GetHitMontage(EHitDirection Direction) const
{
	if (EnemyMontageData)
	{
		UAnimMontage* Montage = EnemyMontageData->GetHitMontage(EnemyType, Direction);
		if (Montage)
		{
			return Montage;
		}
	}

	return nullptr;
}

UAnimMontage* ABaseEnemy::GetDieMontage() const
{
	if (EnemyMontageData)
	{
		UAnimMontage* Montage = EnemyMontageData->GetDieMontage(EnemyType);
		if (Montage)
		{
			return Montage;
		}
	}

	return nullptr;
}

UAnimMontage* ABaseEnemy::GetAttackMontage(FGameplayTagContainer TargetTags) const
{
	if (EnemyMontageData)
	{
		UAnimMontage* Montage = EnemyMontageData->GetAttackMontage(EnemyType, TargetTags);
		if (Montage)
		{
			return Montage;
		}
	}

	return nullptr;
}

bool ABaseEnemy::CheckCurrentBehavior(EAIBehavior NewBehavior)
{
	if (CurrentBehavior == NewBehavior)
	{
		return true;
	}
	return false;
}
