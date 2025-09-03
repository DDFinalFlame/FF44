#include "BossLightningProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "MonsterTags.h"
#include "DrawDebugHelpers.h"

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
    Movement->InitialSpeed = 2000.f;
    Movement->MaxSpeed = 2000.f;
    Movement->ProjectileGravityScale = 0.f;
    Movement->bRotationFollowsVelocity = true;
    Movement->bInitialVelocityInLocalSpace = true;
    //����źó�� ���� ���ؼ� �ʿ�.
    Movement->bIsHomingProjectile = true;
    Movement->HomingAccelerationMagnitude = 8000.f; // ȸ��/�Ѵ� ��. �ʿ�� ����
}

void ABossLightningProjectile::BeginPlay()
{
    Super::BeginPlay();

    if (Movement)
    {
        Movement->SetVelocityInLocalSpace(FVector::ForwardVector * Movement->InitialSpeed);
        Movement->Activate(true);
    }


}


void ABossLightningProjectile::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

  /*  if (bDebugDraw && Collision)
    {
        const FVector Center = Collision->GetComponentLocation();
        const float Radius = Collision->GetScaledSphereRadius();

        DrawDebugSphere(GetWorld(), Center, Radius, 12, FColor::Green, false, -1.f, 0, DebugLineThickness);

        if (Movement)
        {
            const FVector Velocity = Movement->Velocity;
            DrawDebugLine(GetWorld(), Center, Center + Velocity.GetSafeNormal() * 150.f, FColor::Cyan, false, -1.f, 0, DebugLineThickness);
        }
    }*/
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
        // �������� ������ �̺�Ʈ ����
        FGameplayEventData Payload;
        Payload.EventTag = MonsterTags::Event_Boss_P2_WeakPointDestroyed;
        Payload.Instigator = this;
        Payload.Target = TargetBoss.Get();
        Payload.EventMagnitude = DamageValue;

        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetBoss.Get(), Payload.EventTag, Payload);

        Destroy();
    }
}