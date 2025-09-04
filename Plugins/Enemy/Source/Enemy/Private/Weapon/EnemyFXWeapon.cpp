// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/EnemyFXWeapon.h"
#include "Particles/ParticleSystemComponent.h"

#include "BaseEnemy.h"

AEnemyFXWeapon::AEnemyFXWeapon()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>("Root");

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("MeshComponent");
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>("Particle");
	ParticleSystemComponent->SetupAttachment(MeshComponent);

}

void AEnemyFXWeapon::EquipWeapon()
{
	WeaponCollision->SetWeaponMesh(MeshComponent);
	SecondWeaponCollision->SetWeaponMesh(MeshComponent);

	if (ABaseEnemy* OwnerCharacter = Cast<ABaseEnemy>(GetOwner()))
	{
		WeaponCollision->AddIgnoredActor(OwnerCharacter);
		SecondWeaponCollision->AddIgnoredActor(OwnerCharacter);
	}


}

void AEnemyFXWeapon::ActivateCollision()
{
	Super::ActivateCollision();

	// FX ÇÃ·¹ÀÌ
	ParticleSystemComponent->ActivateSystem();

}

void AEnemyFXWeapon::DeactivateCollision()
{
	Super::DeactivateCollision();
}
