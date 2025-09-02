#include "Weapon/SwordWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"

ASwordWeapon::ASwordWeapon()
{
    SwordMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SwordMesh"));
    RootComponent = SwordMesh;

    SwordHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("SwordHitbox"));
    SwordHitbox->SetupAttachment(SwordMesh);
    SwordHitbox->SetBoxExtent(FVector(5, 30, 5));

    RegisterHitbox(SwordHitbox);
}