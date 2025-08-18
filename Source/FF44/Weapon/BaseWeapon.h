// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseWeapon.generated.h"

class ABasePlayer;
class UStaticMeshComponent;
class USphereComponent;

UCLASS()
class FF44_API ABaseWeapon : public AActor
{
	GENERATED_BODY()
	
public:
	ABaseWeapon();

public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp,	AActor* OtherActor, 
									  UPrimitiveComponent* OtherComp,int32 OtherBodyIndex, 
									  bool bFromSweep, const FHitResult& SweepResult);

///////////////////////////////////////////////////////////////////////////////////////////
///										Components										///
///////////////////////////////////////////////////////////////////////////////////////////
protected:
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "Mesh")
	UStaticMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "Collision")
	USphereComponent* WeaponCollision;

protected:
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "Player")
	ABasePlayer* OwnerPlayer;

public:
	void SetPlayer(ABasePlayer* _OwnerPlayer) { _OwnerPlayer = OwnerPlayer; }
	ABasePlayer* GetPlayer() { return OwnerPlayer; }
};
