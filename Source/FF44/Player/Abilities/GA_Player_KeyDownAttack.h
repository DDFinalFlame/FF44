#pragma once

#include "CoreMinimal.h"
#include "Player/Abilities/GA_Player_Attack.h"
#include "GA_Player_KeyDownAttack.generated.h"



UCLASS()
class FF44_API UGA_Player_KeyDownAttack : public UGA_Player_Attack
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Section")
	FName EndSectionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notify")
	FName BeginAttackName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notify")
	FName EndAttackName;

public:
	UGA_Player_KeyDownAttack();

protected:
	virtual void CommitExecute(const FGameplayAbilitySpecHandle Handle,
							   const FGameplayAbilityActorInfo* ActorInfo,
							   const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
							const FGameplayAbilityActorInfo* ActorInfo,
							const FGameplayAbilityActivationInfo ActivationInfo,
							bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	virtual void BeginNotify(FName NotifyName, const FBranchingPointNotifyPayload& Payload);

	UFUNCTION()
	virtual void EndNotify(FName NotifyName, const FBranchingPointNotifyPayload& Payload);

	UFUNCTION()
	virtual void EndAttack();
};
