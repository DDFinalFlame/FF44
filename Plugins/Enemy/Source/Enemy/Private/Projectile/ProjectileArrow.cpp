#include "Projectile/ProjectileArrow.h"

AProjectileArrow::AProjectileArrow()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AProjectileArrow::BeginPlay()
{
	Super::BeginPlay();
	
}

void AProjectileArrow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
