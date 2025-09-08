// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyDefine.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_EnemyWeaponOnOff.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "EnemyWeaponCollision"))
class ENEMY_API UAnimNotifyState_EnemyWeaponOnOff : public UAnimNotifyState
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	bool HasCombo = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	bool bShouldBlend = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	FName CurrentSectionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	FName NextSectionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponType")
	EWeaponType WeaponType = EWeaponType::None;

public:
	UAnimNotifyState_EnemyWeaponOnOff(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
