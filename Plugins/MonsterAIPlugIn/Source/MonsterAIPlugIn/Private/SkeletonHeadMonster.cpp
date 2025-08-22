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
        // 필요 시 Damage도 여기서 덮어쓰기 가능: Weapon->Damage = 12.f;
    }
}