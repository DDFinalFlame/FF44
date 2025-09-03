// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/EnemyFXWeapon.h"

#include "BaseEnemy.h"

AEnemyFXWeapon::AEnemyFXWeapon()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>("Root");

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("MeshComponent");
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
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
