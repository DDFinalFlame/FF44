#include "BossLightningProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "MonsterTags.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "MonsterAttributeSet.h"

ABossLightningProjectile::ABossLightningProjectile()
{
    PrimaryActorTick.bCanEverTick = true;

    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    SetRootComponent(Collision);
    Collision->InitSphereRadius(15.f);
    Collision->SetCollisionProfileName(TEXT("Projectile"));
    Collision->OnComponentBeginOverlap.AddDynamic(this, &ABossLightningProjectile::OnOverlap);

    Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    Movement->UpdatedComponent = Collision;     
    Movement->InitialSpeed = 500.f;
    Movement->MaxSpeed = 2000.f;
    Movement->ProjectileGravityScale = 0.f;
    Movement->bRotationFollowsVelocity = true;
    Movement->bInitialVelocityInLocalSpace = true;
    //유도탄처럼 쓰기 위해서 필요.
    Movement->bIsHomingProjectile = true;
    Movement->HomingAccelerationMagnitude = 8000.f; // 회전/쫓는 힘. 필요시 조절
}

void ABossLightningProjectile::BeginPlay()
{
    Super::BeginPlay();

    if (Movement)
    {
        Movement->SetVelocityInLocalSpace(FVector::ForwardVector * Movement->InitialSpeed);
        Movement->Activate(true);
    }

    if (SpawnSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, SpawnSound, GetActorLocation());
    }

}


void ABossLightningProjectile::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (UAbilitySystemComponent* BossASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetBoss.Get()))
    {
        if (const UMonsterAttributeSet* Attr = Cast<UMonsterAttributeSet>(BossASC->GetAttributeSet(UMonsterAttributeSet::StaticClass())))
        {
            if (Attr->GetHealth() <= 0.f)
            {
                Destroy();
            }
        }
    }

}

void ABossLightningProjectile::InitProjectile(AActor* InTargetBoss, float InDamage)
{
    TargetBoss = InTargetBoss;
    DamageValue = InDamage;

    if (TargetBoss.IsValid())
    {
        USceneComponent* TargetComp = TargetBoss->GetRootComponent();
        Movement->HomingTargetComponent = TargetComp;
    }
}

void ABossLightningProjectile::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!TargetBoss.IsValid()) { Destroy(); return; }

    if (OtherActor == TargetBoss.Get())
    {
        if (HitSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
        }
        // 보스에게 데미지 이벤트 전달
        FGameplayEventData Payload;
        Payload.EventTag = MonsterTags::Event_Boss_P2_WeakPointDestroyed;
        Payload.Instigator = this;
        Payload.Target = TargetBoss.Get();
        Payload.EventMagnitude = DamageValue;

        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetBoss.Get(), Payload.EventTag, Payload);

        Destroy();
    }
}