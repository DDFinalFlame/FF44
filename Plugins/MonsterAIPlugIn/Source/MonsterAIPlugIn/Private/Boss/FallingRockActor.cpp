
#include "Boss/FallingRockActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemBlueprintLibrary.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "MonsterTags.h"

#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Field/FieldSystemComponent.h"
#include "Field/FieldSystemObjects.h"
#include "DrawDebugHelpers.h"

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
    GeoComp->bNotifyCollisions = true;

    GeoComp->SetNotifyBreaks(true); // 브레이크 이벤트
    GeoComp->OnChaosBreakEvent.AddDynamic(this, &AFallingRockActor::OnChaosBreak);
    GeoComp->OnChaosPhysicsCollision.AddDynamic(this, &AFallingRockActor::OnChaosCollision);


    // Hit 이벤트는 GC에서는 OnComponentHit 대신 ChaosPhysicsCollision을 쓰는 편이 안정적
    // GeoComp->SetNotifyRigidBodyCollision(true); // 필요 시

    // (선택) FieldSystem
    FieldSystem = CreateDefaultSubobject<UFieldSystemComponent>(TEXT("FieldSystem"));
    FieldSystem->SetupAttachment(GeoComp);

    // 판정용 히트박스
    HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
    HitBox->SetCanEverAffectNavigation(false);
    HitBox->SetupAttachment(GeoComp);
    HitBox->SetBoxExtent(FVector(40.f));           // 필요에 맞게 조절
    HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    HitBox->SetCollisionObjectType(ECC_WorldDynamic);
    HitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    HitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    HitBox->OnComponentBeginOverlap.AddDynamic(this, &AFallingRockActor::OnHitBoxBeginOverlap);

    GeoComp->SetCanEverAffectNavigation(false);
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

    SetHitBoxActive(true);

    if (bShowPredictedImpact)
    {
        FHitResult PredHit;
        FVector PredPoint;
        if (PredictImpactPoint(PredPoint, PredHit))
        {
           // DrawImpactMarker(PredPoint, PredHit.ImpactNormal);

            // 예측값 캐시
            bHasPredicted = true;
            PredictedPoint = PredPoint;
            PredictedNormal = PredHit.ImpactNormal;

            // 표시용 BP 스폰 (서버 물리 간섭 방지: 클라에서만 생성 권장)
            if (GroundMarkerClass)   //  서버에선 안 만듦
            {
                const FRotator SpawnRot = bAlignMarkerToGround
                    ? FRotationMatrix::MakeFromZ(PredHit.ImpactNormal).Rotator()
                    : FRotator::ZeroRotator;

                FActorSpawnParameters SP;
                SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

                GroundMarkerActor = GetWorld()->SpawnActor<AActor>(
                    GroundMarkerClass, PredPoint, SpawnRot, SP);

                if (GroundMarkerActor)
                {
                    // 표시용이므로 충돌/오버랩 완전 OFF (물리/트레이스 간섭 방지)
                    GroundMarkerActor->SetActorEnableCollision(false);

                    if (MarkerUniformScale != 1.f)
                        GroundMarkerActor->SetActorScale3D(FVector(MarkerUniformScale));
                }
            }
        }
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


    // === 추가: HitBox 기준 초짧은 스윕으로 지면 접촉 보강 ===
    FVector HitPoint;
    if (HitBoxTouchesGround(HitPoint))
    {
        HandleLanded(HitPoint);
        return;
    }

    if (bDebugHitBox && HitBox)
    {
        const FTransform& T = HitBox->GetComponentTransform();
        const FVector Extent = HitBox->GetScaledBoxExtent();

        // 히트박스(초록)
        DrawDebugBox(GetWorld(), T.GetLocation(), Extent, T.GetRotation(),
            FColor::Green, /*bPersistent=*/false, /*LifeTime=*/0.f, /*DepthPri=*/0, /*Thickness=*/2.f);

        // (선택) GC Bounds도 같이 확인(하늘에 남아있는지 디버깅)
        if (GeoComp)
        {
            const auto& C = GeoComp->Bounds;
            DrawDebugBox(GetWorld(), C.Origin, C.BoxExtent, GeoComp->GetComponentQuat(),
                FColor::Cyan, false, 0.f, 0, 1.f);
        }
    }
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


    SetHitBoxActive(false);

    // 표시용 BP 제거
    if (GroundMarkerActor)
    {
        GroundMarkerActor->Destroy();
        GroundMarkerActor = nullptr;
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

    FGameplayEventData Payload;
    Payload.EventTag = MonsterTags::Event_Player_Hit;
    Payload.Instigator = this; 
    Payload.Target = OtherActor;
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OtherActor, Payload.EventTag, Payload);

    

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

//헬퍼함수
void AFallingRockActor::SetHitBoxActive(bool bActive)
{
    if (!HitBox) return;
    bHitBoxActive = bActive;

    if (bActive)
    {
        HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        HitBox->SetGenerateOverlapEvents(true);
        PrimaryActorTick.SetTickFunctionEnable(true);   // 따라가기 계속
    }
    else
    {
        HitBox->SetGenerateOverlapEvents(false);
        HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PrimaryActorTick.SetTickFunctionEnable(false);  // 더 이상 따라갈 필요 X
    }
}



void AFallingRockActor::HandleLanded(const FVector& At)
{
    if (bHasLanded) return;
    bHasLanded = true;

    if (ImpactSound) 
        UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, At);
    // 착지 FX (전용서버 제외, 바닥 노멀 정렬)
    if (LandFX && GetNetMode() != NM_DedicatedServer)
    {
        const FRotator FXRot = (bAlignFXToGround && bHasPredicted)
            ? FRotationMatrix::MakeFromZ(PredictedNormal).Rotator()
            : FRotator::ZeroRotator;

        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), LandFX, At, FXRot, FVector(1.f),
            /*bAutoDestroy=*/true, /*bAutoActivate=*/true, ENCPoolMethod::AutoRelease);
    }

    // 표시용 BP 제거
    if (GroundMarkerActor)
    {
        GroundMarkerActor->Destroy();
        GroundMarkerActor = nullptr;
    }

    ApplyFractureFieldAt(At);

    if (bDestroyOnGroundHit)
    {
        // 조각 굳히고 제거
        if (GeoComp)
        {
            GeoComp->SetSimulatePhysics(false);
            GeoComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
        SetLifeSpan(FMath::Max(0.01f, DestroyDelayOnGround));
    }
}

bool AFallingRockActor::SweepGroundHit(FHitResult& OutHit) const
{
    if (!GeoComp) return false;

    UWorld* World = GetWorld();
    if (!World) return false;

    const FBoxSphereBounds& B = GeoComp->Bounds;

    // 바운드 바닥 근처에서 아래로 짧게 스윕
    const FVector Start = B.Origin;
    const FVector End = Start + FVector(0, 0, -(B.BoxExtent.Z + 20.f)); // 여유 20
    const float Radius = FMath::Clamp(B.SphereRadius * 0.25f, 8.f, 60.f);

    FCollisionQueryParams Params(SCENE_QUERY_STAT(RockGroundSweep), false, this);
    FCollisionObjectQueryParams ObjParams;
    ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);   // 바닥만

    return World->SweepSingleByObjectType(
        OutHit,
        Start, End,
        FQuat::Identity,
        ObjParams,
        FCollisionShape::MakeSphere(Radius),
        Params
    );
}


bool AFallingRockActor::HitBoxTouchesGround(FVector& OutHitPoint) const
{
    if (!HitBox) return false;
    UWorld* World = GetWorld();
    if (!World) return false;

    const FTransform T = HitBox->GetComponentTransform();
    const FVector   Center = T.GetLocation();
    const FVector   Extent = HitBox->GetScaledBoxExtent();

    // 월드 Down 고정 (회전 무시)
    const FVector   Down = FVector(0.f, 0.f, -1.f);

    // 바닥면 추정 위치(히트박스 바닥)보다 "조금 위"에서 시작해 "조금 아래"로 내리쳐
    // -> 겹침 상태 시작을 피해서 히트를 얻기 쉽게 함
    const float     UpPadCm = 5.f;   // 시작점을 살짝 위로
    const float     DownSpan = 12.f;  // 아래로 짧게
    const FVector   Foot = Center + Down * (-Extent.Z);           // 바닥 중앙
    const FVector   Start = Foot - Down * UpPadCm;                    // 위에서 시작
    const FVector   End = Start + Down * DownSpan;                  // 아래로 짧게
    const float     Radius = FMath::Clamp(Extent.Size() * 0.08f, 6.f, 24.f);

    FCollisionObjectQueryParams ObjParams;
    ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);
    ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic); // 이동식 바닥(Platform) 등 대비

  

    FCollisionQueryParams Params(SCENE_QUERY_STAT(RockHitBoxGround), false, this);
    // 필요시 자기 자신/컴포넌트 무시
    Params.AddIgnoredActor(this);
    if (GroundMarkerActor)                 // 표시용 BP 무시
        Params.AddIgnoredActor(GroundMarkerActor);
    Params.bTraceComplex = false;

    FHitResult Hit;
    bool bHit = World->SweepSingleByObjectType(
        Hit,
        Start, End,
        FQuat::Identity,
        ObjParams,
        FCollisionShape::MakeSphere(Radius),
        Params
    );

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    if (bDebugHitBox)
    {
        DrawDebugSphere(World, Start, Radius, 12, FColor::Yellow, false, 0.f);
        DrawDebugLine(World, Start, End, FColor::Yellow, false, 0.f, 0, 1.f);
        if (bHit) DrawDebugPoint(World, Hit.ImpactPoint, 8.f, FColor::Red, false, 0.f);
    }
#endif

    if (bHit)
    {
        OutHitPoint = Hit.ImpactPoint;
        return true;
    }

    // === 보강: 아주 작은 구를 바닥 아래에 겹치기 검사 (스윕이 겹침 시작으로 실패한 경우 대비) ===
    {
        const FVector Probe = Foot + Down * 2.f; // 바닥 바로 아래 2cm 지점
        const float   ProbeR = 6.f;

        bool bOverlap = World->OverlapAnyTestByObjectType(
            Probe,
            FQuat::Identity,
            ObjParams,
            FCollisionShape::MakeSphere(ProbeR),
            Params
        );

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
        if (bDebugHitBox)
        {
            DrawDebugSphere(World, Probe, ProbeR, 12, bOverlap ? FColor::Red : FColor::White, false, 0.f);
        }
#endif

        if (bOverlap)
        {
            OutHitPoint = Probe; // 근사값이면 충분
            return true;
        }
    }

    return false;
}



bool AFallingRockActor::PredictImpactPoint(FVector& OutPoint, FHitResult& OutHit) const
{
    UWorld* World = GetWorld();
    if (!World || !GeoComp) return false;

    // 현재 바운드 기준으로 충분히 위→아래로 길게 긁어서 바닥 찾기
    const FBoxSphereBounds& B = GeoComp->Bounds;

    const FVector Start = B.Origin + FVector(0, 0, B.BoxExtent.Z + 50.f); // 바운드 조금 위
    const FVector End = Start + FVector(0, 0, -PredictMaxDownTrace);

    // 둥근 스윕(바운드 스케일 고려). 너무 크면 과도히 빨리 맞아서 줄임
    const float Radius = FMath::Clamp(B.SphereRadius * 0.3f, 8.f, 60.f);

    FCollisionObjectQueryParams Obj;
    Obj.AddObjectTypesToQuery(ECC_WorldStatic);
    Obj.AddObjectTypesToQuery(ECC_WorldDynamic); // 이동식 플랫폼 대비

    FCollisionQueryParams Params(SCENE_QUERY_STAT(RockPredictGround), false, this);
    Params.AddIgnoredActor(this);

    // 스윕이 과하다면 LineTrace로 바꿔도 됨(정확도는 비슷)
    const bool bHit = World->SweepSingleByObjectType(
        OutHit,
        Start, End,
        FQuat::Identity,
        Obj,
        FCollisionShape::MakeSphere(Radius),
        Params
    );

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    if (bDebugHitBox)
    {
        DrawDebugLine(World, Start, End, FColor::Yellow, false, 2.f, 0, 1.f);
        DrawDebugSphere(World, Start, Radius, 12, FColor::Yellow, false, 2.f);
    }
#endif

    if (!bHit) return false;

    OutPoint = OutHit.ImpactPoint;
    return true;
}

void AFallingRockActor::DrawImpactMarker(const FVector& At, const FVector& Normal) const
{
    UWorld* World = GetWorld();
    if (!World) return;

    // 1) 바닥 포인트에 박스/스피어
    DrawDebugSphere(World, At, PredictMarkerSize, 16, PredictMarkerColor, false, PredictMarkerLife);

    // 2) 노멀 방향으로 화살표(착지 면 방향 확인)
    const FVector Tip = At + Normal * (PredictMarkerSize * 2.f);
    DrawDebugDirectionalArrow(World, At, Tip, PredictMarkerSize * 2.f, PredictMarkerColor, false, PredictMarkerLife, 0, 2.f);

    // 3) (선택) 원형 가이드(수평)
    DrawDebugCircle(World, At + FVector(0, 0, 2.f), PredictMarkerSize * 1.5f, 32, PredictMarkerColor, false, PredictMarkerLife, 0, 1.f, FVector(1, 0, 0), FVector(0, 1, 0), false);

    // 4) 텍스트
    DrawDebugString(World, At + FVector(0, 0, PredictMarkerSize + 10.f), TEXT("Predicted Impact"), nullptr, PredictMarkerColor, PredictMarkerLife, false);
}