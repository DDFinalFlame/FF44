#include "Projectile/ProjectileBase.h"

#include "AbilitySystemBlueprintLibrary.h"

AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// Component 세팅
	if (!RootComponent)
	{
		RootComponent = CreateDefaultSubobject<USceneComponent>("Root");
	}
	/* 충돌 Component **/
	if (!CapsuleComponent)
	{
		CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
		//CapsuleComponent->BodyInstance.SetCollisionProfileName(TEXT("Projectile"));
		CapsuleComponent->SetupAttachment(RootComponent);
		// 충돌 처리
		CapsuleComponent->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);
	}

	/* Mesh **/
	if (!MeshComponent)
	{
		MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("MeshComponent");
		MeshComponent->SetupAttachment(CapsuleComponent);
		MeshComponent->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
	}


	/* 이동 Component **/
	if (!ProjectileMovementComponent)
	{
		ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
		ProjectileMovementComponent->SetUpdatedComponent(CapsuleComponent);
		// 기타 값 세팅은 BP 설정값 가지고 Construnction 시 설정
	}

	// 발사체 속성
	/* life time **/
	InitialLifeSpan = 4.0f;
}

void AProjectileBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->InitialSpeed = InitialSpeed;
		ProjectileMovementComponent->MaxSpeed = MaxSpeed;
		ProjectileMovementComponent->bAutoActivate = true;
		ProjectileMovementComponent->bRotationFollowsVelocity = false;
		ProjectileMovementComponent->ProjectileGravityScale = ProjectileGravityScale;
	}
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
	CapsuleComponent->IgnoreActorWhenMoving(GetOwner(), true);
	ProjectileMovementComponent->Velocity = ShootDirection * ProjectileMovementComponent->InitialSpeed;
}

void AProjectileBase::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this) { return; }
	if (OtherActor == GetOwner()) { return; }

	FGameplayEventData EventData;
	
	EventData.EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Hit"));
	EventData.Instigator = GetOwner();
	EventData.Target = OtherActor;

	/* HitResult를 TargetData로 포장 **/ 
	EventData.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(Hit);

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		OtherActor,
		EventData.EventTag,
		EventData
	);

	Destroy();
}

