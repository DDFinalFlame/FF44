// Fill out your copyright notice in the Description page of Project Settings.


#include "BossCharacter.h"
#include "MonsterBaseWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

ABossCharacter::ABossCharacter()
{
}

void ABossCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority()) return;
    if (!WeaponClass || Weapon) return;

    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    Weapon = GetWorld()->SpawnActor<AMonsterBaseWeapon>(WeaponClass, FTransform::Identity, Params);
    if (Weapon)
    {
        Weapon->Init(this, GetMesh(), WeaponAttachSocketName);
    }
}