
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
    GeoComp->bNotifyCollisions = true;

    GeoComp->SetNotifyBreaks(true); // �극��ũ �̺�Ʈ
    GeoComp->OnChaosBreakEvent.AddDynamic(this, &AFallingRockActor::OnChaosBreak);
    GeoComp->OnChaosPhysicsCollision.AddDynamic(this, &AFallingRockActor::OnChaosCollision);


    // Hit �̺�Ʈ�� GC������ OnComponentHit ��� ChaosPhysicsCollision�� ���� ���� ������
    // GeoComp->SetNotifyRigidBodyCollision(true); // �ʿ� ��

    // (����) FieldSystem
    FieldSystem = CreateDefaultSubobject<UFieldSystemComponent>(TEXT("FieldSystem"));
    FieldSystem->SetupAttachment(GeoComp);

    // ������ ��Ʈ�ڽ�
    HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
    HitBox->SetCanEverAffectNavigation(false);
    HitBox->SetupAttachment(GeoComp);
    HitBox->SetBoxExtent(FVector(40.f));           // �ʿ信 �°� ����
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
        // �Ʒ��� �ʱ� �ӵ� �ο�
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

            // ������ ĳ��
            bHasPredicted = true;
            PredictedPoint = PredPoint;
            PredictedNormal = PredHit.ImpactNormal;

            // ǥ�ÿ� BP ���� (���� ���� ���� ����: Ŭ�󿡼��� ���� ����)
            if (GroundMarkerClass)   //  �������� �� ����
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
                    // ǥ�ÿ��̹Ƿ� �浹/������ ���� OFF (����/Ʈ���̽� ���� ����)
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


    // === �߰�: HitBox ���� ��ª�� �������� ���� ���� ���� ===
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

        // ��Ʈ�ڽ�(�ʷ�)
        DrawDebugBox(GetWorld(), T.GetLocation(), Extent, T.GetRotation(),
            FColor::Green, /*bPersistent=*/false, /*LifeTime=*/0.f, /*DepthPri=*/0, /*Thickness=*/2.f);

        // (����) GC Bounds�� ���� Ȯ��(�ϴÿ� �����ִ��� �����)
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

// === Chaos �浹 �ݹ� ===
void AFallingRockActor::OnChaosCollision(const FChaosPhysicsCollisionInfo& Info)
{
    const FVector P = Info.Location;

    // FX
    if (ImpactFX) UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactFX, P);


    SetHitBoxActive(false);

    // ǥ�ÿ� BP ����
    if (GroundMarkerActor)
    {
        GroundMarkerActor->Destroy();
        GroundMarkerActor = nullptr;
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

    FGameplayEventData Payload;
    Payload.EventTag = MonsterTags::Event_Player_Hit;
    Payload.Instigator = this; 
    Payload.Target = OtherActor;
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OtherActor, Payload.EventTag, Payload);

    

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

//�����Լ�
void AFallingRockActor::SetHitBoxActive(bool bActive)
{
    if (!HitBox) return;
    bHitBoxActive = bActive;

    if (bActive)
    {
        HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        HitBox->SetGenerateOverlapEvents(true);
        PrimaryActorTick.SetTickFunctionEnable(true);   // ���󰡱� ���
    }
    else
    {
        HitBox->SetGenerateOverlapEvents(false);
        HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PrimaryActorTick.SetTickFunctionEnable(false);  // �� �̻� ���� �ʿ� X
    }
}



void AFallingRockActor::HandleLanded(const FVector& At)
{
    if (bHasLanded) return;
    bHasLanded = true;

    if (ImpactSound) 
        UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, At);
    // ���� FX (���뼭�� ����, �ٴ� ��� ����)
    if (LandFX && GetNetMode() != NM_DedicatedServer)
    {
        const FRotator FXRot = (bAlignFXToGround && bHasPredicted)
            ? FRotationMatrix::MakeFromZ(PredictedNormal).Rotator()
            : FRotator::ZeroRotator;

        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), LandFX, At, FXRot, FVector(1.f),
            /*bAutoDestroy=*/true, /*bAutoActivate=*/true, ENCPoolMethod::AutoRelease);
    }

    // ǥ�ÿ� BP ����
    if (GroundMarkerActor)
    {
        GroundMarkerActor->Destroy();
        GroundMarkerActor = nullptr;
    }

    ApplyFractureFieldAt(At);

    if (bDestroyOnGroundHit)
    {
        // ���� ������ ����
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

    // �ٿ�� �ٴ� ��ó���� �Ʒ��� ª�� ����
    const FVector Start = B.Origin;
    const FVector End = Start + FVector(0, 0, -(B.BoxExtent.Z + 20.f)); // ���� 20
    const float Radius = FMath::Clamp(B.SphereRadius * 0.25f, 8.f, 60.f);

    FCollisionQueryParams Params(SCENE_QUERY_STAT(RockGroundSweep), false, this);
    FCollisionObjectQueryParams ObjParams;
    ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);   // �ٴڸ�

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

    // ���� Down ���� (ȸ�� ����)
    const FVector   Down = FVector(0.f, 0.f, -1.f);

    // �ٴڸ� ���� ��ġ(��Ʈ�ڽ� �ٴ�)���� "���� ��"���� ������ "���� �Ʒ�"�� ������
    // -> ��ħ ���� ������ ���ؼ� ��Ʈ�� ��� ���� ��
    const float     UpPadCm = 5.f;   // �������� ��¦ ����
    const float     DownSpan = 12.f;  // �Ʒ��� ª��
    const FVector   Foot = Center + Down * (-Extent.Z);           // �ٴ� �߾�
    const FVector   Start = Foot - Down * UpPadCm;                    // ������ ����
    const FVector   End = Start + Down * DownSpan;                  // �Ʒ��� ª��
    const float     Radius = FMath::Clamp(Extent.Size() * 0.08f, 6.f, 24.f);

    FCollisionObjectQueryParams ObjParams;
    ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);
    ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic); // �̵��� �ٴ�(Platform) �� ���

  

    FCollisionQueryParams Params(SCENE_QUERY_STAT(RockHitBoxGround), false, this);
    // �ʿ�� �ڱ� �ڽ�/������Ʈ ����
    Params.AddIgnoredActor(this);
    if (GroundMarkerActor)                 // ǥ�ÿ� BP ����
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

    // === ����: ���� ���� ���� �ٴ� �Ʒ��� ��ġ�� �˻� (������ ��ħ �������� ������ ��� ���) ===
    {
        const FVector Probe = Foot + Down * 2.f; // �ٴ� �ٷ� �Ʒ� 2cm ����
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
            OutHitPoint = Probe; // �ٻ簪�̸� ���
            return true;
        }
    }

    return false;
}



bool AFallingRockActor::PredictImpactPoint(FVector& OutPoint, FHitResult& OutHit) const
{
    UWorld* World = GetWorld();
    if (!World || !GeoComp) return false;

    // ���� �ٿ�� �������� ����� ����Ʒ��� ��� �ܾ �ٴ� ã��
    const FBoxSphereBounds& B = GeoComp->Bounds;

    const FVector Start = B.Origin + FVector(0, 0, B.BoxExtent.Z + 50.f); // �ٿ�� ���� ��
    const FVector End = Start + FVector(0, 0, -PredictMaxDownTrace);

    // �ձ� ����(�ٿ�� ������ ���). �ʹ� ũ�� ������ ���� �¾Ƽ� ����
    const float Radius = FMath::Clamp(B.SphereRadius * 0.3f, 8.f, 60.f);

    FCollisionObjectQueryParams Obj;
    Obj.AddObjectTypesToQuery(ECC_WorldStatic);
    Obj.AddObjectTypesToQuery(ECC_WorldDynamic); // �̵��� �÷��� ���

    FCollisionQueryParams Params(SCENE_QUERY_STAT(RockPredictGround), false, this);
    Params.AddIgnoredActor(this);

    // ������ ���ϴٸ� LineTrace�� �ٲ㵵 ��(��Ȯ���� ���)
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

    // 1) �ٴ� ����Ʈ�� �ڽ�/���Ǿ�
    DrawDebugSphere(World, At, PredictMarkerSize, 16, PredictMarkerColor, false, PredictMarkerLife);

    // 2) ��� �������� ȭ��ǥ(���� �� ���� Ȯ��)
    const FVector Tip = At + Normal * (PredictMarkerSize * 2.f);
    DrawDebugDirectionalArrow(World, At, Tip, PredictMarkerSize * 2.f, PredictMarkerColor, false, PredictMarkerLife, 0, 2.f);

    // 3) (����) ���� ���̵�(����)
    DrawDebugCircle(World, At + FVector(0, 0, 2.f), PredictMarkerSize * 1.5f, 32, PredictMarkerColor, false, PredictMarkerLife, 0, 1.f, FVector(1, 0, 0), FVector(0, 1, 0), false);

    // 4) �ؽ�Ʈ
    DrawDebugString(World, At + FVector(0, 0, PredictMarkerSize + 10.f), TEXT("Predicted Impact"), nullptr, PredictMarkerColor, PredictMarkerLife, false);
}