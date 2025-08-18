#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PArrow.generated.h"

UCLASS()
class ENEMY_API APArrow : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, Category = Movement)
	UProjectileMovementComponent* ProjectileMovementComponent;


public:	
	APArrow();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

// Set value at Fire
public:
	/* �߻��� �� ����, �ӵ� �ʱ�ȭ **/
	void FireInDirection(const FVector& ShootDirection);
};
