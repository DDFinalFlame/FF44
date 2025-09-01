
#include "Boss/FallingRockActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "MonsterTags.h"

#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Field/FieldSystemComponent.h"
#include "Field/FieldSystemObjects.h"

static bool IsGroundHit(const FChaosPhysicsCollisionInfo& Info)
{
    return Info.OtherComponent
        && Info.OtherComponent->GetCollisionObjectType() == ECC_WorldStatic;
}


AFallingRockActor::AFallingRockActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // === Geometry Collection ��Ʈ ===
    GeoComp = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeoComp"));
    SetRootComponent(GeoComp);

    // �浹/����
    GeoComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GeoComp->SetCollisionObjectType(ECC_WorldDynamic);
    GeoComp->SetCollisionResponseToAllChannels(ECR_Block);
    GeoComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    GeoComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    GeoComp->BodyInstance.bUseCCD = true;

    GeoComp->SetSimulatePhysics(true);
    GeoComp->SetEnableGravity(true);

    // ����ó/���� �̺�Ʈ ����
    GeoComp->SetNotifyBreaks(true); // �극��ũ �̺�Ʈ
    GeoComp->OnChaosBreakEvent.AddDynamic(this, &AFallingRockActor::OnChaosBreak);
    GeoComp->OnChaosPhysicsCollision.AddDynamic(this, &AFallingRockActor::OnChaosCollision);

    GeoComp->SetCanEverAffectNavigation(false);
    if (HitBox) HitBox->SetCanEverAffectNavigation(false);
    // Hit �̺�Ʈ�� GC������ OnComponentHit ��� ChaosPhysicsCollision�� ���� ���� ������
    // GeoComp->SetNotifyRigidBodyCollision(true); // �ʿ� ��

    // (����) FieldSystem
    FieldSystem = CreateDefaultSubobject<UFieldSystemComponent>(TEXT("FieldSystem"));
    FieldSystem->SetupAttachment(GeoComp);

    // ������ ��Ʈ�ڽ�
    HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
    HitBox->SetupAttachment(GeoComp);
    HitBox->SetBoxExtent(FVector(40.f));           // �ʿ信 �°� ����
    HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    HitBox->SetCollisionObjectType(ECC_WorldDynamic);
    HitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    HitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    HitBox->OnComponentBeginOverlap.AddDynamic(this, &AFallingRockActor::OnHitBoxBeginOverlap);
  
    if (!ByCallerDamageTag.IsValid())
    {
        ByCallerDamageTag = MonsterTags::Data_Drop_Damage;
    }
}

void AFallingRockActor::BeginPlay()
{
    Super::BeginPlay();

    if (LifeSeconds > 0.f)
    {
        SetLifeSpan(LifeSeconds);
    }

    if (InitialDownSpeed > 0.f && GeoComp && GeoComp->IsSimulatingPhysics())
    {
        // �Ʒ��� �ʱ� �ӵ� �ο�
        const FVector InitVel(0.f, 0.f, -InitialDownSpeed);
        GeoComp->SetPhysicsLinearVelocity(InitVel);
    }
}


void AFallingRockActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!GeoComp || !HitBox) return;
    if (bHasLanded) return; // ���� �Ŀ� ���� ���� �� �Ϸ���

    // Chaos�� �� ������ �����ϴ� Bounds ���
    const FBoxSphereBounds& B = GeoComp->Bounds;

    // ȸ���� GeoComp ���ʹϾ� �״��, ��ġ�� �ٿ�� �߽�
    const FQuat Rot = GeoComp->GetComponentQuat();
    const FTransform Xform(Rot, B.Origin);

    // �ڷ���Ʈ ������� ��ġ(���� ���� X) �� ���� ������ ����
    HitBox->SetWorldTransform(Xform, /*bSweep=*/false, /*OutHit=*/nullptr, ETeleportType::TeleportPhysics);

    // �ڽ� ũ��(half-extent). Bounds.BoxExtent ��ü�� half�̹Ƿ� �״�� ���, ��¦ ���
    HitBox->SetBoxExtent(B.BoxExtent * 0.6f, /*bUpdateOverlaps=*/false);

    // UE5.6: ���� ���� ȣ�� (�⺻��: PendingOverlaps=nullptr, bDoNotifies=true)
    HitBox->UpdateOverlaps();
}

void AFallingRockActor::SetDamageInstigator(AActor* InInstigator)
{
    DamageInstigator = InInstigator;
    SetOwner(InInstigator);
}

// === Chaos �浹 �ݹ� ===
void AFallingRockActor::OnChaosCollision(const FChaosPhysicsCollisionInfo& Info)
{
    const FVector P = Info.Location;

    // FX
    if (ImpactFX) UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactFX, P);
    if (ImpactSound) UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, P);

    // �ٴڿ� ����� ���� ����
    if (!bHasLanded && IsGroundHit(Info))
    {
        bHasLanded = true;
        ApplyFractureFieldAt(P);

        if (bDestroyOnGroundHit)
        {
            FTimerHandle Th;
            GetWorldTimerManager().SetTimer(Th, FTimerDelegate::CreateWeakLambda(this, [this]()
                {
                    if (GeoComp)
                    {
                        GeoComp->SetSimulatePhysics(false);
                        GeoComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                    }
                    SetLifeSpan(FMath::Max(0.01f, DestroyDelayOnGround));
                }), 0.03f, false);
        }
    }
}

// === �극��ũ(���� �и�) �ݹ� ===
void AFallingRockActor::OnChaosBreak(const FChaosBreakEvent& BreakEvent)
{
    // �ʿ� �� ����ȭ�� �� �߰� ����Ʈ/����/���ھ ���� ��������.
    // ��: UE_LOG(LogTemp, Warning, TEXT("GC Break Pieces=%d"), BreakEvent.Component->GetNumElements());
}

void AFallingRockActor::OnHitBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    UE_LOG(LogTemp, Warning, TEXT("[Rock] Overlap with %s (Auth=%d)"),
        *GetNameSafe(OtherActor), HasAuthority() ? 1 : 0);

    if (!HasAuthority()) return;
    if (!OtherActor || OtherActor == this) return;
    if (ShouldIgnore(OtherActor)) return;

    if (bOncePerActor)
    {
        if (AlreadyHitSet.Contains(OtherActor)) return;
        AlreadyHitSet.Add(OtherActor);
    }

    ApplyDamageTo(OtherActor, SweepResult);

    // ���� �� Ư�� ���ǿ����� ����� ����ȭ�� �ϰ� ������ ���⼭�� �ʵ� ���� ����
    // ApplyFractureFieldAt(SweepResult.ImpactPoint);
}

bool AFallingRockActor::ShouldIgnore(AActor* OtherActor) const
{
    if (!OtherActor) return true;

    if (bHasLanded) return true;

    if (bIgnoreOwnerAndInstigatorTeam)
    {
        if (OtherActor == GetOwner()) return true;
        if (DamageInstigator.IsValid() && OtherActor == DamageInstigator.Get()) return true;
        // �� �� ������ ���Ŵٸ� ���⿡�� ���� ���̸� return true;
    }

    return false;
}

UAbilitySystemComponent* AFallingRockActor::GetASCFromActor(AActor* Actor) const
{
    if (!Actor) return nullptr;
    return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
}

void AFallingRockActor::ApplyDamageTo(AActor* Target, const FHitResult& OptionalHit)
{
    if (!GE_Damage) return;

    UAbilitySystemComponent* TargetASC = GetASCFromActor(Target);
    if (!TargetASC) return;

    FGameplayEffectContextHandle Ctx = TargetASC->MakeEffectContext();

    AActor* InstigatorActor = DamageInstigator.IsValid() ? DamageInstigator.Get() : this;
    APawn* InstigatorPawn = Cast<APawn>(InstigatorActor);
    AController* InstigatorController = InstigatorPawn ? InstigatorPawn->GetController() : nullptr;

    Ctx.AddInstigator(InstigatorActor, InstigatorController);
    Ctx.AddSourceObject(this);
    if (OptionalHit.bBlockingHit || OptionalHit.bStartPenetrating || OptionalHit.Component.IsValid())
    {
        Ctx.AddHitResult(OptionalHit);
    }

    FGameplayEffectSpecHandle Spec = TargetASC->MakeOutgoingSpec(GE_Damage, 1.f, Ctx);
    if (!Spec.IsValid()) return;

    if (ByCallerDamageTag.IsValid())
    {
        Spec.Data->SetSetByCallerMagnitude(ByCallerDamageTag, DamageMagnitude);
    }

    TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}

/** �ʵ�(ExternalClusterStrain)�� ����ó�� ���� */
void AFallingRockActor::ApplyFractureFieldAt(const FVector& Center)
{
    if (!GeoComp) return;

    // RadialFalloff(Scalar) -> ToInteger -> ExternalClusterStrain �� �����ϴ� ����� �Ϲ���
    URadialFalloff* Radial = NewObject<URadialFalloff>();
    if (!Radial) return;

    Radial->Magnitude = FractureStrength;   // ����(Ŭ������ ��Ʈ���ο� ���� ����)
    Radial->MinRange = 0.f;
    Radial->MaxRange = FractureRadius;
    Radial->Default = 0.f;
    Radial->Radius = FractureRadius;
    Radial->Position = Center;
    Radial->Falloff = EFieldFalloffType::Field_FallOff_None; // �ʿ� �� Linear ������ ����

    // Chaos_Field_ExternalClusterStrain : Ŭ������ �ı� ����
    GeoComp->ApplyPhysicsField(
        true,
        EGeometryCollectionPhysicsTypeEnum::Chaos_ExternalClusterStrain,
        nullptr,
        Radial
    );
}