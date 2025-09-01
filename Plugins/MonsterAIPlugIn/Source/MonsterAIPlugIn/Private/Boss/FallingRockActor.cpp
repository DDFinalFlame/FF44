
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

    // === Geometry Collection 루트 ===
    GeoComp = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeoComp"));
    SetRootComponent(GeoComp);

    // 충돌/물리
    GeoComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GeoComp->SetCollisionObjectType(ECC_WorldDynamic);
    GeoComp->SetCollisionResponseToAllChannels(ECR_Block);
    GeoComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    GeoComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    GeoComp->BodyInstance.bUseCCD = true;

    GeoComp->SetSimulatePhysics(true);
    GeoComp->SetEnableGravity(true);

    // 프랙처/물리 이벤트 수신
    GeoComp->SetNotifyBreaks(true); // 브레이크 이벤트
    GeoComp->OnChaosBreakEvent.AddDynamic(this, &AFallingRockActor::OnChaosBreak);
    GeoComp->OnChaosPhysicsCollision.AddDynamic(this, &AFallingRockActor::OnChaosCollision);

    GeoComp->SetCanEverAffectNavigation(false);
    if (HitBox) HitBox->SetCanEverAffectNavigation(false);
    // Hit 이벤트는 GC에서는 OnComponentHit 대신 ChaosPhysicsCollision을 쓰는 편이 안정적
    // GeoComp->SetNotifyRigidBodyCollision(true); // 필요 시

    // (선택) FieldSystem
    FieldSystem = CreateDefaultSubobject<UFieldSystemComponent>(TEXT("FieldSystem"));
    FieldSystem->SetupAttachment(GeoComp);

    // 판정용 히트박스
    HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
    HitBox->SetupAttachment(GeoComp);
    HitBox->SetBoxExtent(FVector(40.f));           // 필요에 맞게 조절
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
        // 아래로 초기 속도 부여
        const FVector InitVel(0.f, 0.f, -InitialDownSpeed);
        GeoComp->SetPhysicsLinearVelocity(InitVel);
    }
}


void AFallingRockActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!GeoComp || !HitBox) return;
    if (bHasLanded) return; // 착지 후엔 굳이 갱신 안 하려면

    // Chaos가 매 프레임 갱신하는 Bounds 사용
    const FBoxSphereBounds& B = GeoComp->Bounds;

    // 회전은 GeoComp 쿼터니언 그대로, 위치는 바운드 중심
    const FQuat Rot = GeoComp->GetComponentQuat();
    const FTransform Xform(Rot, B.Origin);

    // 텔레포트 방식으로 배치(물리 스윕 X) → 이후 오버랩 재계산
    HitBox->SetWorldTransform(Xform, /*bSweep=*/false, /*OutHit=*/nullptr, ETeleportType::TeleportPhysics);

    // 박스 크기(half-extent). Bounds.BoxExtent 자체가 half이므로 그대로 사용, 살짝 축소
    HitBox->SetBoxExtent(B.BoxExtent * 0.6f, /*bUpdateOverlaps=*/false);

    // UE5.6: 인자 없이 호출 (기본값: PendingOverlaps=nullptr, bDoNotifies=true)
    HitBox->UpdateOverlaps();
}

void AFallingRockActor::SetDamageInstigator(AActor* InInstigator)
{
    DamageInstigator = InInstigator;
    SetOwner(InInstigator);
}

// === Chaos 충돌 콜백 ===
void AFallingRockActor::OnChaosCollision(const FChaosPhysicsCollisionInfo& Info)
{
    const FVector P = Info.Location;

    // FX
    if (ImpactFX) UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactFX, P);
    if (ImpactSound) UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, P);

    // 바닥에 닿았을 때만 착지
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

// === 브레이크(파편 분리) 콜백 ===
void AFallingRockActor::OnChaosBreak(const FChaosBreakEvent& BreakEvent)
{
    // 필요 시 파편화될 때 추가 이펙트/사운드/스코어링 등을 넣으세요.
    // 예: UE_LOG(LogTemp, Warning, TEXT("GC Break Pieces=%d"), BreakEvent.Component->GetNumElements());
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

    // 무기 등 특정 조건에서만 ‘즉시 파편화’ 하고 싶으면 여기서도 필드 적용 가능
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
        // 팀 비교 로직을 쓰신다면 여기에서 같은 편이면 return true;
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

/** 필드(ExternalClusterStrain)로 프랙처를 유도 */
void AFallingRockActor::ApplyFractureFieldAt(const FVector& Center)
{
    if (!GeoComp) return;

    // RadialFalloff(Scalar) -> ToInteger -> ExternalClusterStrain 로 전달하는 방법이 일반적
    URadialFalloff* Radial = NewObject<URadialFalloff>();
    if (!Radial) return;

    Radial->Magnitude = FractureStrength;   // 강도(클러스터 스트레인에 직접 매핑)
    Radial->MinRange = 0.f;
    Radial->MaxRange = FractureRadius;
    Radial->Default = 0.f;
    Radial->Radius = FractureRadius;
    Radial->Position = Center;
    Radial->Falloff = EFieldFalloffType::Field_FallOff_None; // 필요 시 Linear 등으로 변경

    // Chaos_Field_ExternalClusterStrain : 클러스터 파괴 유도
    GeoComp->ApplyPhysicsField(
        true,
        EGeometryCollectionPhysicsTypeEnum::Chaos_ExternalClusterStrain,
        nullptr,
        Radial
    );
}