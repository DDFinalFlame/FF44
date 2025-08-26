#include "BaseEnemy.h"
#include "EnemyRotationComponent.h"
#include "MonsterAttributeSet.h"
#include "GAS/EnemyAttributeSet.h"
#include "MonsterDefinition.h"
#include "MonsterStatRow.h"
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
	if (WeaponClass)
	{
		// GetWorld()로 액터 생성
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		Weapon = GetWorld()->SpawnActor<AEnemyBaseWeapon>(
			WeaponClass,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (Weapon)
		{
			Weapon->SetOwner(this);
			Weapon->EquipWeapon();
		}
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
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, static_cast<int32>(INDEX_NONE), this));
		}
	}

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

bool ABaseEnemy::RequestAbilityByTag(FGameplayTag AbilityTag)
{
	if (!AbilitySystemComponent)
	{
		return false;
	}

	FGameplayTagContainer TagContainer(AbilityTag);
	return AbilitySystemComponent->TryActivateAbilitiesByTag(TagContainer);
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

void ABaseEnemy::ActivateWeaponCollision()
{
	Weapon->ActivateCollision();
}

void ABaseEnemy::DeactivateWeaponCollision()
{
	Weapon->DeactivateCollision();
}

