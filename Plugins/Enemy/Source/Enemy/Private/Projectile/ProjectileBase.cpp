#include "Projectile/ProjectileBase.h"

AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// Component ����
	if (!RootComponent)
	{
		RootComponent = CreateDefaultSubobject<USceneComponent>("Root");
	}
	/* �浹 Component **/
	if (!CapsuleComponent)
	{
		CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
		CapsuleComponent->SetupAttachment(RootComponent);
	}

	/* Mesh **/
	if (!MeshComponent)
	{
		MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("MeshComponent");
		MeshComponent->SetupAttachment(CapsuleComponent);
		MeshComponent->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
	}


	/* �̵� Component **/
	if (!ProjectileMovementComponent)
	{
		ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
		ProjectileMovementComponent->SetUpdatedComponent(CapsuleComponent);
		ProjectileMovementComponent->InitialSpeed = 2500.0f;
		ProjectileMovementComponent->MaxSpeed = 3000.0f;
		ProjectileMovementComponent->bRotationFollowsVelocity = false;
		ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
	}

	// �߻�ü �Ӽ�
	/* life time **/
	InitialLifeSpan = 4.0f;
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

}

void AProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectileBase::FireInDirection(const FVector& ShootDirection)
{
	ProjectileMovementComponent->Velocity = ShootDirection * ProjectileMovementComponent->InitialSpeed;
}

