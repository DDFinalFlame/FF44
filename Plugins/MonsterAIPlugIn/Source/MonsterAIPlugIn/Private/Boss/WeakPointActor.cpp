// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss/WeakPointActor.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "MonsterTags.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "BossLightningProjectile.h"
#include "MonsterAttributeSet.h"
#include "AbilitySystemComponent.h"

//디버깅용
#include "Engine/TargetPoint.h"


AWeakPointActor::AWeakPointActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Mesh->SetNotifyRigidBodyCollision(true);
    Mesh->SetSimulatePhysics(false);

    // 간단히 Hit 이벤트로 부서지게 연결(원하시면 Overlap, HP 시스템 등으로 확장)
    Mesh->OnComponentHit.AddDynamic(this, &AWeakPointActor::OnMeshHit);
}


void AWeakPointActor::BeginPlay()
{
    Super::BeginPlay();

    // 전용 서버에서는 재생 안 함
    if (GetNetMode() == NM_DedicatedServer) return;

    if (SpawnSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            SpawnSound,
            GetActorLocation()
        );
    }
}

void AWeakPointActor::InitializeWeakPoint(AActor* _BossActor, float _DamageToBoss)
{
    OwnerBoss = _BossActor;
    DamageToBossOnBreak = _DamageToBoss;
}

void AWeakPointActor::OnMeshHit(UPrimitiveComponent* _HitComp, AActor* _OtherActor,
    UPrimitiveComponent* _OtherComp, FVector _NormalImpulse, const FHitResult& _Hit)
{

}

void AWeakPointActor::HandleDelayedBreak()
{
    // 1) 나이아가라 이펙트
    if (BreakVFX)
    {
        const FVector SpawnAt = (CachedImpactPoint.IsNearlyZero() ? GetActorLocation() : CachedImpactPoint) + VFXOffset;
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), BreakVFX, SpawnAt, GetActorRotation());
    }

    // 2) 보스 방향으로 번개 투사체 스폰(선택)
    SpawnProjectileTowardBoss();

    // 3) 최종 알림 + 제거
    BreakAndNotify();
}

void AWeakPointActor::NotifyHitByPlayerWeapon(const FHitResult& Hit, AActor* Attacker)
{
    if (bPendingBreak || bBroken) return;
    bPendingBreak = true;

    CachedImpactPoint = FVector(Hit.ImpactPoint).IsNearlyZero() ? GetActorLocation() : FVector(Hit.ImpactPoint);
    CachedAttacker = Attacker;

    // 랜덤 지연
    const float Delay = FMath::FRandRange(BreakDelayMin, BreakDelayMax);

    GetWorldTimerManager().SetTimer(
        BreakTimerHandle,
        this, &AWeakPointActor::HandleDelayedBreak,
        Delay, false
    );

}

void AWeakPointActor::SpawnProjectileTowardBoss()
{
    if (!ProjectileClass || !OwnerBoss.IsValid()) return;
    // 보스 체력 확인.
    if (UAbilitySystemComponent* BossASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerBoss.Get()))
    {
        if (const UMonsterAttributeSet* Attr = Cast<UMonsterAttributeSet>(BossASC->GetAttributeSet(UMonsterAttributeSet::StaticClass())))
        {
            if (Attr->GetHealth() <= 0.f)
            {
                // 보스가 이미 죽었으면 발사 안 함
                return;
            }
        }
    }

    UWorld* World = GetWorld();
    if (!World) return;

    const FVector Start = CachedImpactPoint.IsNearlyZero() ? GetActorLocation() : CachedImpactPoint;
    const FRotator Rot = (OwnerBoss->GetActorLocation() - Start).Rotation();
    const FTransform Xf(Rot, Start);

    ABossLightningProjectile* P = Cast<ABossLightningProjectile>(
        UGameplayStatics::BeginDeferredActorSpawnFromClass(
            this, ProjectileClass, Xf, ESpawnActorCollisionHandlingMethod::AlwaysSpawn, this));

    if (!P) return;

    // BeginPlay/Overlap 전에 미리 초기화
    P->InitProjectile(OwnerBoss.Get(), DamageToBossOnBreak);

    UGameplayStatics::FinishSpawningActor(P, Xf);
}


void AWeakPointActor::BreakAndNotify()
{
    if (bBroken) return;
    bBroken = true;

    Destroy(); // 파괴
}
