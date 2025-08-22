#include "HeadbuttWeapon.h"
#include "Components/BoxComponent.h"

AHeadbuttWeapon::AHeadbuttWeapon()
{
    HeadHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("HeadHitbox"));
    RootComponent = HeadHitbox;
    HeadHitbox->SetBoxExtent(FVector(15, 15, 15));
    RegisterHitbox(HeadHitbox);
}