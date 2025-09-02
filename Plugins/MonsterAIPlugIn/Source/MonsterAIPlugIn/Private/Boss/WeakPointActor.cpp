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

    // ������ Hit �̺�Ʈ�� �μ����� ����(���Ͻø� Overlap, HP �ý��� ������ Ȯ��)
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
    // ���⼭�� � Ÿ���̵� ������ �μ����� �����Դϴ�.
    // �ʿ��ϸ� "�÷��̾� ���⸸" ����ϴ� ���� ������ �߰��ϼ���.
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
        Payload.EventMagnitude = DamageToBossOnBreak; // ���ط��� �̺�Ʈ�� ����

        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
            OwnerBoss.Get(), Payload.EventTag, Payload);
    }

    Destroy(); // �ı�
}