#include "BaseEnemy.h"

#include "EnemyRotationComponent.h"

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
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(PerformAttackAbility, 1, 0));
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

bool ABaseEnemy::RequestAttack()
{
	if (!AbilitySystemComponent || !PerformAttackAbility)
	{
		return false;
	}

	FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(PerformAttackAbility);
	if (!AbilitySpec)
	{
		return false; // Ability가 부여되지 않은 경우
	}

	return AbilitySystemComponent->TryActivateAbility(AbilitySpec->Handle);
}

