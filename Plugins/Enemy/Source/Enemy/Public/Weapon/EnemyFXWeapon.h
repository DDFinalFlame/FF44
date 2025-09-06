// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyBaseWeapon.h"
#include "EnemyFXWeapon.generated.h"

/**
 * 
 */

UCLASS()
class ENEMY_API AEnemyFXWeapon : public AEnemyBaseWeapon
{
	GENERATED_BODY()


protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* MeshAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystemComponent* ParticleSystemComponent;

public:
	AEnemyFXWeapon();

public:
	virtual void EquipWeapon() override;
	virtual void ActivateCollision() override;
	virtual void DeactivateCollision() override;

};
