// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss/WeakPointActor.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "MonsterTags.h"

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

void AWeakPointActor::InitializeWeakPoint(AActor* _BossActor, float _DamageToBoss)
{
    OwnerBoss = _BossActor;
    DamageToBossOnBreak = _DamageToBoss;
}

void AWeakPointActor::OnMeshHit(UPrimitiveComponent* _HitComp, AActor* _OtherActor,
    UPrimitiveComponent* _OtherComp, FVector _NormalImpulse, const FHitResult& _Hit)
{
    // 여기서는 어떤 타격이든 맞으면 부서지는 예시입니다.
    // 필요하면 "플레이어 무기만" 허용하는 필터 조건을 추가하세요.
    BreakAndNotify();
}

void AWeakPointActor::BreakAndNotify()
{
    if (bBroken) return;
    bBroken = true;

    if (OwnerBoss.IsValid())
    {
        FGameplayEventData Payload;
        Payload.EventTag = MonsterTags::Event_Boss_P2_WeakPointDestroyed;
        Payload.Instigator = this;
        Payload.Target = OwnerBoss.Get();
        Payload.EventMagnitude = DamageToBossOnBreak; // 피해량을 이벤트로 전달

        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
            OwnerBoss.Get(), Payload.EventTag, Payload);
    }

    Destroy(); // 파괴
}