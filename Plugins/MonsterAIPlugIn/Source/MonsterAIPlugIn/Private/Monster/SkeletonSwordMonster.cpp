#include "Monster/SkeletonSwordMonster.h"
#include "Weapon/MonsterBaseWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "MonsterAttributeSet.h"

ASkeletonSwordMonster::ASkeletonSwordMonster()
{
}

void ASkeletonSwordMonster::BeginPlay()
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
        // 필요 시 Damage 덮어쓰기: Weapon->Damage = 20.f;
    }
}



void ASkeletonSwordMonster::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!HasAuthority()) return;

    // === HP 체크 후 사운드 재생 ===
    if (AbilitySystemComponent)
    {
        if (const UMonsterAttributeSet* Attr = Cast<UMonsterAttributeSet>(AttributeSet))
        {
            if (!bDeathSoundPlayed && Attr->GetHealth() <= 0.f)
            {
                bDeathSoundPlayed = true;

                if (DeathSound)
                {
                    UGameplayStatics::PlaySoundAtLocation(
                        this,
                        DeathSound,
                        GetActorLocation()
                    );
                }
            }
        }
    }
}