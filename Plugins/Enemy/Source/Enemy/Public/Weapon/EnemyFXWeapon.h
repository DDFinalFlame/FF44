// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyBaseWeapon.h"
#include "EnemyFXWeapon.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
/**
 * 
 */
UCLASS()
class ENEMY_API AEnemyFXWeapon : public AEnemyBaseWeapon
{
	GENERATED_BODY()


protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UNiagaraComponent* NiagaraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UNiagaraSystem* NiagaraSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SocketName = "Hand_Middle_Socket";


public:
	AEnemyFXWeapon();
protected:
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	virtual void EquipWeapon() override;
	virtual void ActivateCollision() override;
	virtual void DeactivateCollision() override;

};
