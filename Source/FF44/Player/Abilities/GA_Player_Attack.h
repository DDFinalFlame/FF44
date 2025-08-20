#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Player_Attack.generated.h"

UCLASS()
class FF44_API UGA_Player_Attack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Player_Attack();

protected:
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "Player")
	class ABasePlayer* OwnerPlayer;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "Player")
	class ABaseWeapon* OwnerWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
	class UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
	float MontagePlayRate = 1.0f;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
								 const FGameplayAbilityActorInfo* ActorInfo, 
								 const FGameplayAbilityActivationInfo ActivationInfo,
								 const FGameplayEventData* TriggerEventData) override;	

	virtual void CommitExecute(const FGameplayAbilitySpecHandle Handle,
							   const FGameplayAbilityActorInfo* ActorInfo,
							   const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
							const FGameplayAbilityActorInfo* ActorInfo, 
							const FGameplayAbilityActivationInfo ActivationInfo,
							bool bReplicateEndAbility, bool bWasCancelled) override;
};
