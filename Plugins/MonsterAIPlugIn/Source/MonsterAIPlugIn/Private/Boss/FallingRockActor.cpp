
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

AFallingRockActor::AFallingRockActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);

    // �浹 + ���� ����
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Mesh->SetCollisionObjectType(ECC_WorldDynamic);

    // �⺻ ����: ����� ���, ���� ������(�÷��̾� ����)
    Mesh->SetCollisionResponseToAllChannels(ECR_Block);
    Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

    // ������ ����
    Mesh->SetSimulatePhysics(true);
    Mesh->SetEnableGravity(true);
    Mesh->BodyInstance.bUseCCD = true;

    // Hit �̺�Ʈ ���� (���ŷ �浹)
    Mesh->SetNotifyRigidBodyCollision(true);
    Mesh->OnComponentHit.AddDynamic(this, &AFallingRockActor::OnMeshHit);

    HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
    HitBox->SetupAttachment(Mesh);
    HitBox->SetBoxExtent(FVector(40.f));           // �ʿ信 �°� ����
    HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    HitBox->SetCollisionObjectType(ECC_WorldDynamic);
    HitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    HitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    HitBox->OnComponentBeginOverlap.AddDynamic(this, &AFallingRockActor::OnHitBoxBeginOverlap);
    // �⺻ ByCaller �±�
    if (!ByCallerDamageTag.IsValid())
    {
        // ������Ʈ���� ���� ������ �±׸����� �ٲټ���. (��: "Data.Damage")
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

    // �ʱ� �ϰ� �ӵ�(���Ͻø� ���)
    if (InitialDownSpeed > 0.f && Mesh && Mesh->IsSimulatingPhysics())
    {
        Mesh->SetPhysicsLinearVelocity(FVector(0.f, 0.f, -InitialDownSpeed));
    }
}

void AFallingRockActor::SetDamageInstigator(AActor* InInstigator)
{
    DamageInstigator = InInstigator;
    SetOwner(InInstigator); // ���ؽ�Ʈ ������ ����
}

void AFallingRockActor::OnMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse,
    const FHitResult& Hit)
{
    // �ڱ� �ڽ�/��ȿ üũ
    if (!OtherComp || OtherActor == this) return;

    // ���� �Ǵ� ����� ���ŷ �浹�� ������ ����
    // (�ʿ��ϸ� OtherComp�� ObjectType���� WorldStatic/WorldDynamic üũ ����)
    FVector ImpactPoint = Hit.ImpactPoint;

    // ����Ʈ/����
    if (ImpactFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactFX, ImpactPoint);
    }
    if (ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, ImpactPoint);
    }

    bHasLanded = true;

    // ���鿡 ������ ���߰� �ı�(�ɼ�)
    if (bDestroyOnGroundHit)
    {
        // �� �̻� Ƣ�� �ʰ� ����/�浹 ��Ȱ��
        if (Mesh)
        {
            Mesh->SetSimulatePhysics(false);
            Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
        SetLifeSpan(FMath::Max(0.01f, DestroyDelayOnGround));
    }
}

void AFallingRockActor::OnHitBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    UE_LOG(LogTemp, Warning, TEXT("[Rock] Overlap with %s (Auth=%d)"),
        *GetNameSafe(OtherActor), HasAuthority() ? 1 : 0);

    if (!HasAuthority()) return;               // ���������� ó��
    if (!OtherActor || OtherActor == this) return;
    if (ShouldIgnore(OtherActor)) return;

    if (bOncePerActor)
    {
        if (AlreadyHitSet.Contains(OtherActor)) return;
        AlreadyHitSet.Add(OtherActor);
    }

    ApplyDamageTo(OtherActor, SweepResult);

    // 1ȸ�� �����̸� ���⼭ �ٷ� �ı��ص� ��(����)
    // Destroy();
}


bool AFallingRockActor::ShouldIgnore(AActor* OtherActor) const
{
    if (!OtherActor) return true;

    if (bHasLanded) return true;

    // �ڱ���/��ȯ�� ���� ���� ����
    if (bIgnoreOwnerAndInstigatorTeam)
    {
        if (OtherActor == GetOwner()) return true;
        if (DamageInstigator.IsValid() && OtherActor == DamageInstigator.Get()) return true;

        // �� �ý����� �ִٸ� ���⿡�� �� ���Ͽ� ���� ���̸� ����
        // e.g., IGenericTeamAgentInterface* etc...
    }

    return false;
}

UAbilitySystemComponent* AFallingRockActor::GetASCFromActor(AActor* Actor) const
{
    if (!Actor) return nullptr;

    // ǥ�� ����
    return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
}



void AFallingRockActor::ApplyDamageTo(AActor* Target, const FHitResult& OptionalHit)
{
    if (!GE_Damage) return;

    UAbilitySystemComponent* TargetASC = GetASCFromActor(Target);
    if (!TargetASC) return;

    // Ÿ�� ASC �������� ���ؽ�Ʈ ����
    FGameplayEffectContextHandle Ctx = TargetASC->MakeEffectContext();
    // Instigator: ������ ����(����) �Ǵ� �� ���� �ڽ�
    AActor* InstigatorActor = DamageInstigator.IsValid() ? DamageInstigator.Get() : this;
    APawn* InstigatorPawn = Cast<APawn>(InstigatorActor);
    AController* InstigatorController = InstigatorPawn ? InstigatorPawn->GetController() : nullptr;

    Ctx.AddInstigator(InstigatorActor, InstigatorController);
    Ctx.AddSourceObject(this);
    if (OptionalHit.bBlockingHit || OptionalHit.bStartPenetrating || OptionalHit.Component.IsValid())
    {
        Ctx.AddHitResult(OptionalHit);
    }

    FGameplayEffectSpecHandle Spec = TargetASC->MakeOutgoingSpec(GE_Damage, /*Level=*/1.f, Ctx);
    if (!Spec.IsValid()) return;

    // ByCaller ����
    if (ByCallerDamageTag.IsValid())
    {
        Spec.Data->SetSetByCallerMagnitude(ByCallerDamageTag, DamageMagnitude);
    }

    TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}