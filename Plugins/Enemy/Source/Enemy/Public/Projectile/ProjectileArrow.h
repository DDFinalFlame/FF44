#pragma once

#include "CoreMinimal.h"
#include "Projectile/ProjectileBase.h"
#include "GameFramework/Actor.h"
#include "ProjectileArrow.generated.h"

UCLASS()
class ENEMY_API AProjectileArrow : public AProjectileBase
{
	GENERATED_BODY()


public:	
	AProjectileArrow();


protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};
