#pragma once

#include "CoreMinimal.h"
#include "EnemyWeaponCollisionComponent.h"
#include "GameFramework/Actor.h"
#include "EnemyBaseWeapon.generated.h"

class UEnemyWeaponCollisionComponent;

UCLASS()
class ENEMY_API AEnemyBaseWeapon : public AActor
{
	GENERATED_BODY()

protected:
	/* �ݸ��� 1 **/
	UPROPERTY(VisibleAnywhere)
	UEnemyWeaponCollisionComponent* WeaponCollision;

	/* �ݸ��� 2 **/
	UPROPERTY(VisibleAnywhere)
	UEnemyWeaponCollisionComponent* SecondWeaponCollision;


public:
	AEnemyBaseWeapon();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	void EquipWeapon();
	void ActivateCollision();
	void DeactivateCollision();

public:
	FORCEINLINE bool IsAttackSuccessful() const { return WeaponCollision->IsAttackSuccessful() || SecondWeaponCollision->IsAttackSuccessful();}
};
