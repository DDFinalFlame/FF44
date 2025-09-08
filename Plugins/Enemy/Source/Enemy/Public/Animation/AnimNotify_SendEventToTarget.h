// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_SendEventToTarget.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "TriggerTargetEvent"))
class ENEMY_API UAnimNotify_SendEventToTarget : public UAnimNotify
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS | TAG")
	FGameplayTag EventTag;

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
