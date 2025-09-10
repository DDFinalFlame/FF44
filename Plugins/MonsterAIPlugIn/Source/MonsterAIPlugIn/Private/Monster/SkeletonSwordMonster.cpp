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
        // �ʿ� �� Damage �����: Weapon->Damage = 20.f;
    }
}



void ASkeletonSwordMonster::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!HasAuthority()) return;

    // === HP üũ �� ���� ��� ===
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