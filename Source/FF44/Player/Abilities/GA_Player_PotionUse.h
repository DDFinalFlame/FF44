#pragma once

#include "CoreMinimal.h"
#include "Player/Abilities/GA_Player_ItemUse.h"
#include "GA_Player_PotionUse.generated.h"

class AActor;
class UGameplayEffect;

UCLASS()
class FF44_API UGA_Player_PotionUse : public UGA_Player_ItemUse
{
	GENERATED_BODY()

public:
	UGA_Player_PotionUse();
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion | Object")
	TSubclassOf<AActor> PotionClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Potion | Object")
	TObjectPtr<AActor> Potion;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion | Socket")
	FName PotionSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion | Notify")
	FName DrinkEndNotify;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion | Sound")
	class USoundBase* StartDrinkSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion | Sound")
	class USoundBase* EndDrinkSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion | Sound")
	class USoundBase* HealSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Potion | Effect")
	TSubclassOf<UGameplayEffect> HealEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Potion | Niagara")
	class UNiagaraSystem* HealNiagara;

protected:
	virtual void CommitExecute(const FGameplayAbilitySpecHandle Handle,
							   const FGameplayAbilityActorInfo* ActorInfo,
							   const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
							const FGameplayAbilityActorInfo* ActorInfo,
							const FGameplayAbilityActivationInfo ActivationInfo,
							bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	void OnNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& Payload) override;
	void OnBlendIn() override;

	bool FindItem() override;
};
