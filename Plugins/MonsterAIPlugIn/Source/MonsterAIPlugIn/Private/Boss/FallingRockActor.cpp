
#include "Boss/FallingRockActor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"

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

    // ��������Ʈ ���ε�
    Mesh->OnComponentHit.AddDynamic(this, &AFallingRockActor::OnMeshHit);
    Mesh->OnComponentBeginOverlap.AddDynamic(this, &AFallingRockActor::OnMeshBeginOverlap);
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

void AFallingRockActor::OnMeshBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this) return;

    // ĳ����/�÷��̾� ���� ? ���⼭ ������ or GameplayEvent ���� ���� �� �ֽ��ϴ�.
    // ����(�Ϲ� ������):
    // UGameplayStatics::ApplyDamage(OtherActor, 10.f, nullptr, this, UDamageType::StaticClass());

    // GAS �̺�Ʈ�� �˸��� �ʹٸ�(������Ʈ �±׿� ���� ����):
    // #include "AbilitySystemBlueprintLibrary.h"
    // #include "GameplayTagContainer.h"
    // FGameplayEventData Payload;
    // Payload.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Boss.RockHit"));
    // Payload.Instigator = this;
    // Payload.Target = OtherActor;
    // UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OtherActor, Payload.EventTag, Payload);

    // �ʿ� ��, �÷��̾ ������ �ڽ� �ı�:
    // Destroy();
}