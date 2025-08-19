#include "SkeletonHeadMonster.h"
#include "MonsterBaseWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

ASkeletonHeadMonster::ASkeletonHeadMonster()
{

}

void ASkeletonHeadMonster::BeginPlay()
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
        // �ʿ� �� Damage�� ���⼭ ����� ����: Weapon->Damage = 12.f;
    }
}