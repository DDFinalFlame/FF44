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

//������
#include "Engine/TargetPoint.h"


AWeakPointActor::AWeakPointActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Mesh->SetNotifyRigidBodyCollision(true);
    Mesh->SetSimulatePhysics(false);

    // ������ Hit �̺�Ʈ�� �μ����� ����(���Ͻø� Overlap, HP �ý��� ������ Ȯ��)
    Mesh->OnComponentHit.AddDynamic(this, &AWeakPointActor::OnMeshHit);
}


void AWeakPointActor::BeginPlay()
{
    Super::BeginPlay();

    // ���� ���������� ��� �� ��
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
    // 1) ���̾ư��� ����Ʈ
    if (BreakVFX)
    {
        const FVector SpawnAt = (CachedImpactPoint.IsNearlyZero() ? GetActorLocation() : CachedImpactPoint) + VFXOffset;
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), BreakVFX, SpawnAt, GetActorRotation());
    }

    // 2) ���� �������� ���� ����ü ����(����)
    SpawnProjectileTowardBoss();

    // 3) ���� �˸� + ����
    BreakAndNotify();
}

void AWeakPointActor::NotifyHitByPlayerWeapon(const FHitResult& Hit, AActor* Attacker)
{
    if (bPendingBreak || bBroken) return;
    bPendingBreak = true;

    CachedImpactPoint = FVector(Hit.ImpactPoint).IsNearlyZero() ? GetActorLocation() : FVector(Hit.ImpactPoint);
    CachedAttacker = Attacker;

    // ���� ����
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
    // ���� ü�� Ȯ��.
    if (UAbilitySystemComponent* BossASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerBoss.Get()))
    {
        if (const UMonsterAttributeSet* Attr = Cast<UMonsterAttributeSet>(BossASC->GetAttributeSet(UMonsterAttributeSet::StaticClass())))
        {
            if (Attr->GetHealth() <= 0.f)
            {
                // ������ �̹� �׾����� �߻� �� ��
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

    // BeginPlay/Overlap ���� �̸� �ʱ�ȭ
    P->InitProjectile(OwnerBoss.Get(), DamageToBossOnBreak);

    UGameplayStatics::FinishSpawningActor(P, Xf);
}


void AWeakPointActor::BreakAndNotify()
{
    if (bBroken) return;
    bBroken = true;

    Destroy(); // �ı�
}
