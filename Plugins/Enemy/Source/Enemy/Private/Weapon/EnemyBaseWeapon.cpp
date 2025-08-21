#include "Weapon/EnemyBaseWeapon.h"

#include "BaseEnemy.h"
#include "GameFramework/Character.h"
#include "Weapon/EnemyWeaponCollisionComponent.h"

AEnemyBaseWeapon::AEnemyBaseWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	/* 컴포넌트 초기화 */

	WeaponCollision = CreateDefaultSubobject<UEnemyWeaponCollisionComponent>(TEXT("MainCollision"));
	WeaponCollision->PrimaryComponentTick.bCanEverTick = true;
	WeaponCollision->PrimaryComponentTick.TickInterval = 0.01f;

	SecondWeaponCollision = CreateDefaultSubobject<UEnemyWeaponCollisionComponent>(TEXT("SecondCollision"));
	SecondWeaponCollision->PrimaryComponentTick.bCanEverTick = true;
	SecondWeaponCollision->PrimaryComponentTick.TickInterval = 0.01f;
}

void AEnemyBaseWeapon::BeginPlay()
{
	Super::BeginPlay();

}

void AEnemyBaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemyBaseWeapon::EquipWeapon()
{
	if (ABaseEnemy* OwnerCharacter = Cast<ABaseEnemy>(GetOwner()))
	{
		WeaponCollision->SetWeaponMesh(OwnerCharacter->GetMesh());
		SecondWeaponCollision->SetWeaponMesh(OwnerCharacter->GetMesh());

		WeaponCollision->AddIgnoredActor(OwnerCharacter);
		SecondWeaponCollision->AddIgnoredActor(OwnerCharacter);
	}
}

void AEnemyBaseWeapon::ActivateCollision()
{
	WeaponCollision->ActivateCollision();
	SecondWeaponCollision->ActivateCollision();
}

void AEnemyBaseWeapon::DeactivateCollision()
{
	WeaponCollision->DeactivateCollision();
	SecondWeaponCollision->DeactivateCollision();
}
