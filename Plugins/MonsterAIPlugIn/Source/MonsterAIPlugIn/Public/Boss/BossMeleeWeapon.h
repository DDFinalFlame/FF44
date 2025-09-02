#pragma once

#include "CoreMinimal.h"
#include "Weapon/MonsterBaseWeapon.h"
#include "BossMeleeWeapon.generated.h"

class UBoxComponent;
UCLASS()
class MONSTERAIPLUGIN_API ABossMeleeWeapon : public AMonsterBaseWeapon
{
	GENERATED_BODY()
public:
	ABossMeleeWeapon();

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FVector BoxExtent = FVector(15.f, 15.f, 15.f);

protected:
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* Hitbox = nullptr;
	
};
