#pragma once

#include "CoreMinimal.h"
#include "Player/Abilities/GA_Player_Attack.h"
#include "GA_Player_Attack_Combo.generated.h"

struct FBranchingPointNotifyPayload;
struct FGameplayTag;

UCLASS()
class FF44_API UGA_Player_Attack_Combo : public UGA_Player_Attack
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Notify")
	FName CollisionNotifyName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Notify")
	FName ComboNotifyName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tags")
	FGameplayTag ComboEnabledTag;


protected:
	virtual void CommitExecute(const FGameplayAbilitySpecHandle Handle,
							   const FGameplayAbilityActorInfo* ActorInfo,
							   const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
							const FGameplayAbilityActorInfo* ActorInfo,
							const FGameplayAbilityActivationInfo ActivationInfo,
							bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void OnEnableAttack(FName NotifyName, const FBranchingPointNotifyPayload& Payload);

	UFUNCTION()
	void OnDisableAttack(FName NotifyName, const FBranchingPointNotifyPayload& Payload);
};
