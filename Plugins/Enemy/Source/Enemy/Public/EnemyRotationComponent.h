// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyRotationComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ENEMY_API UEnemyRotationComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* TargetActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TargetLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShouldRotate = true;

	/* 회전 속도 ( 1초에 180도 회전 )**/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
	float RotationSpeed = 180.f; 

public:
	UEnemyRotationComponent();

public:
	FORCEINLINE void SetTargetLocation(const FVector& InLocation) { TargetLocation = InLocation; }
	FORCEINLINE void ToggleShouldRotate(const bool bRotate) { bShouldRotate = bRotate; }
	/* WILL BE DEPRECATED **/
	FORCEINLINE void SetTargetActor(AActor* InActor) { TargetActor = InActor; }

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	bool IsVectorValidForUse(const FVector& Vec, float Tolerance = KINDA_SMALL_NUMBER);

};
