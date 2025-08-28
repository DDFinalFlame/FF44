
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

    // 충돌 + 물리 설정
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Mesh->SetCollisionObjectType(ECC_WorldDynamic);

    // 기본 응답: 월드는 블록, 폰은 오버랩(플레이어 감지)
    Mesh->SetCollisionResponseToAllChannels(ECR_Block);
    Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

    // 물리로 낙하
    Mesh->SetSimulatePhysics(true);
    Mesh->SetEnableGravity(true);
    Mesh->BodyInstance.bUseCCD = true;

    // Hit 이벤트 수신 (블로킹 충돌)
    Mesh->SetNotifyRigidBodyCollision(true);

    // 델리게이트 바인딩
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

    // 초기 하강 속도(원하시면 사용)
    if (InitialDownSpeed > 0.f && Mesh && Mesh->IsSimulatingPhysics())
    {
        Mesh->SetPhysicsLinearVelocity(FVector(0.f, 0.f, -InitialDownSpeed));
    }
}

void AFallingRockActor::OnMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse,
    const FHitResult& Hit)
{
    // 자기 자신/무효 체크
    if (!OtherComp || OtherActor == this) return;

    // 지면 또는 월드와 블로킹 충돌한 것으로 간주
    // (필요하면 OtherComp의 ObjectType으로 WorldStatic/WorldDynamic 체크 가능)
    FVector ImpactPoint = Hit.ImpactPoint;

    // 이펙트/사운드
    if (ImpactFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactFX, ImpactPoint);
    }
    if (ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, ImpactPoint);
    }

    // 지면에 닿으면 멈추고 파괴(옵션)
    if (bDestroyOnGroundHit)
    {
        // 더 이상 튀지 않게 물리/충돌 비활성
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

    // 캐릭터/플레이어 감지 ? 여기서 데미지 or GameplayEvent 등을 보낼 수 있습니다.
    // 예시(일반 데미지):
    // UGameplayStatics::ApplyDamage(OtherActor, 10.f, nullptr, this, UDamageType::StaticClass());

    // GAS 이벤트로 알리고 싶다면(프로젝트 태그에 맞춰 수정):
    // #include "AbilitySystemBlueprintLibrary.h"
    // #include "GameplayTagContainer.h"
    // FGameplayEventData Payload;
    // Payload.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Boss.RockHit"));
    // Payload.Instigator = this;
    // Payload.Target = OtherActor;
    // UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OtherActor, Payload.EventTag, Payload);

    // 필요 시, 플레이어에 맞으면 자신 파괴:
    // Destroy();
}