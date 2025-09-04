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

	NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		NiagaraSystem,         // Niagara 시스템 애셋
		MeshComponent,       // 부모 컴포넌트
		"Hand_Middle_Socket",             // 소켓 이름 (없으면 NAME_None)
		FVector::ZeroVector,   // 상대 위치
		FRotator::ZeroRotator, // 상대 회전
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

		//// Boss 소켓에 붙이기
		//FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
		//AttachToComponent(OwnerCharacter->GetMesh(), AttachmentRules, FName("Hand_L_Socket"));
	}


}

void AEnemyFXWeapon::ActivateCollision()
{
	Super::ActivateCollision();

	// FX 플레이
	if (!NiagaraComponent)
	{
		int a = 0;
	}
	else
	{
		NiagaraComponent->Activate(true);

	}
}

void AEnemyFXWeapon::DeactivateCollision()
{
	Super::DeactivateCollision();
}
