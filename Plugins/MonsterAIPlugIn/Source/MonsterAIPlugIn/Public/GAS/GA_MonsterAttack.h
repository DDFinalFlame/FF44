#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_MonsterAttack.generated.h"

/*
 Ability 부여 및 발동 
 */
UCLASS()
class MONSTERAIPLUGIN_API UGA_MonsterAttack : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_MonsterAttack();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	FName AttackKey;
};
