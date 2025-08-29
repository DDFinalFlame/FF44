// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifyState_EnemyWeaponOnOff.h"
#include "AnimNotifyState_EnemyCheckWeapon.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "EnemyWeaponCheck"))
class ENEMY_API UAnimNotifyState_EnemyCheckWeapon : public UAnimNotifyState_EnemyWeaponOnOff
{
	GENERATED_BODY()

public:
	UAnimNotifyState_EnemyCheckWeapon(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime) override;
};
