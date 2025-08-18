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
		GiveDefaultAbilities();
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

