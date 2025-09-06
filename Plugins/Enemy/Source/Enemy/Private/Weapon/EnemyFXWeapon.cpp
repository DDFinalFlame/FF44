// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/EnemyFXWeapon.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

#include "BaseEnemy.h"
#include "NiagaraFunctionLibrary.h"

AEnemyFXWeapon::AEnemyFXWeapon()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>("Root");

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("MeshComponent");
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);


}

void AEnemyFXWeapon::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	FRotator TargetRotator = GetActorRotation();
	TargetRotator.Yaw += 180.0f;

	NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		NiagaraSystem,         // Niagara �ý��� �ּ�
		MeshComponent,       // �θ� ������Ʈ
		"Hand_Middle_Socket",             // ���� �̸� (������ NAME_None)
		FVector::ZeroVector,   // ��� ��ġ
		TargetRotator, // ��� ȸ��
		EAttachLocation::SnapToTarget,
		false
	);

	if (NiagaraComponent)
	{
		NiagaraComponent->Deactivate();
	}

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

	// FX �÷���
	if (NiagaraComponent)
	{
		NiagaraComponent->Activate(true);

	}
}

void AEnemyFXWeapon::DeactivateCollision()
{
	Super::DeactivateCollision();
}
