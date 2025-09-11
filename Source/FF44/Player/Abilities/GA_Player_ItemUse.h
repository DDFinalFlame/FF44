#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Player_ItemUse.generated.h"

UCLASS()
class FF44_API UGA_Player_ItemUse : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Player_ItemUse();

protected:
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "Player")
	TWeakObjectPtr<class ABasePlayer> OwnerPlayer;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "IC")
	TWeakObjectPtr<class UInventoryComponent> IC;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UnEquipAbility")
	TSubclassOf<class UGameplayAbility> UnEquipAbililtyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
	class UAnimMontage* ItemUseMontage;

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

protected:
	UFUNCTION()
	virtual void OnNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& Payload);

	UFUNCTION()
	virtual void OnNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& Payload);

	UFUNCTION(BlueprintCallable)
	virtual void OnCompleted();

	UFUNCTION(BlueprintCallable)
	virtual void OnBlendIn();

	UFUNCTION(BlueprintCallable)
	virtual void OnBlendOut();

	UFUNCTION(BlueprintCallable)
	virtual void OnInterrupted();

	UFUNCTION(BlueprintCallable)
	virtual void OnCancelled();

	virtual bool FindItem();
	virtual void DontFindItem();

private:
	void SetUnEquipMode();
};
