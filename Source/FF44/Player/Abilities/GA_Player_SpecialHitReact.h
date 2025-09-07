#pragma once

#include "CoreMinimal.h"
#include "Player/Abilities/GA_Player_HitReact.h"
#include "GA_Player_SpecialHitReact.generated.h"

UCLASS()
class FF44_API UGA_Player_SpecialHitReact : public UGA_Player_HitReact
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "React | Wraith")
	UAnimMontage* WraithGrapMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "React | Wraith")
	FName WraithSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "React | Wraith")
	FName GrabMotionWarpingNotify;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "React | Wraith")
	FName EndGrabNotify;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "React | Rampage")
	UAnimMontage* RampageGrapMontage;

protected:
	virtual void CommitExecute(const FGameplayAbilitySpecHandle Handle,
							   const FGameplayAbilityActorInfo* ActorInfo,
							   const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
							const FGameplayAbilityActorInfo* ActorInfo,
							const FGameplayAbilityActivationInfo ActivationInfo,
							bool bReplicateEndAbility, bool bWasCancelled) override;

	void OnWraithBoss();
	void OnRampage();
	void OnPlayerGrapMontage();

	virtual void OnBeginNotify(FName NotifyName, const FBranchingPointNotifyPayload& Payload) override;
	virtual void OnEndNotify(FName NotifyName, const FBranchingPointNotifyPayload& Payload) override;
};
