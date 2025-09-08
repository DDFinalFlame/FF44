#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Player_HitReact.generated.h"

class UGameplayEffect;
class UAnimMontage;
class USoundBase;

struct FGameplayEventData;

class ABasePlayer;

UCLASS()
class FF44_API UGA_Player_HitReact : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGameplayEventData EventData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<ABasePlayer> OwnerPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect | Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "React | Sound")
	USoundBase* VoiceSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "React | Sound")
	USoundBase* ArmorSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "React | Montage")
	UAnimMontage* HitMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "React | Montage")
	float MontagePlayRate = 1.0f;

public:
	UGA_Player_HitReact();
	
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
	UFUNCTION()
	virtual void OnBlendInHitReact();

	UFUNCTION()
	virtual void OnBeginNotify(FName NotifyName, const FBranchingPointNotifyPayload& Payload);

	UFUNCTION()
	virtual void OnEndNotify(FName NotifyName, const FBranchingPointNotifyPayload& Payload);
};
