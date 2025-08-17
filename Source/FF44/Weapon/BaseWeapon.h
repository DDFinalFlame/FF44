// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseWeapon.generated.h"

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UStaticMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "Collision")
	USphereComponent* WeaponCollision;


	// 공격력, 공격 속도 등의 스탯을 DB로 관리할 수 있도록 설정



};
