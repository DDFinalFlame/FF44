// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeakPointActor.generated.h"


class UStaticMeshComponent;
class UNiagaraSystem;
class ABossLightningProjectile;

UCLASS()
class MONSTERAIPLUGIN_API AWeakPointActor : public AActor
{
	GENERATED_BODY()
	
public:
    AWeakPointActor();

    // ���� ���� �� �ı� �� ���� ���� ����
    void InitializeWeakPoint(AActor* _BossActor, float _DamageToBoss);

protected:
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;

    UFUNCTION()
    void OnMeshHit(UPrimitiveComponent* _HitComp, AActor* _OtherActor,
        UPrimitiveComponent* _OtherComp, FVector _NormalImpulse, const FHitResult& _Hit);

    UPROPERTY(EditAnywhere, Category = "WeakPoint|Timing")
    float BreakDelayMin = 0.5f;

    /** ���� �� ����� ������ �ִ� ����(��) */
    UPROPERTY(EditAnywhere, Category = "WeakPoint|Timing")
    float BreakDelayMax = 1.0f;

    /** �� ���� Ʈ���ŵǵ��� ���� */
    bool bPendingBreak = false;

    /** ���� �ı� Ÿ�̸� �ڵ� */
    FTimerHandle BreakTimerHandle;

    // �ı� �� ���� ���ط�
    float DamageToBossOnBreak = 10.f;
    // ���⿡�� ���� ��Ʈ ���� ĳ��(���̾ư���/����ü ���������� ���)
    FVector CachedImpactPoint = FVector::ZeroVector;
    TWeakObjectPtr<AActor> CachedAttacker;

    // === VFX / ����ü �ɼ� ===
    UPROPERTY(EditAnywhere, Category = "WeakPoint|VFX")
    UNiagaraSystem* BreakVFX = nullptr;

    UPROPERTY(EditAnywhere, Category = "WeakPoint|VFX")
    FVector VFXOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "WeakPoint|Projectile")
    TSubclassOf<AActor> ProjectileClass; // ���� ����ü ���� Ŭ����

    UPROPERTY(EditAnywhere, Category = "WeakPoint|Projectile")
    float ProjectileSpeed = 2000.f;

    // (���� ������ ���� �� ȸ��/���� ����)
    void SpawnProjectileTowardBoss();
    // ���� ���۷���

    /** ���� ���� �� ȣ��(���⼭ ���̾ư���/����ü�� �̾ ����) */
    void HandleDelayedBreak();

    // �ı� ó��
    void BreakAndNotify();
   
    TWeakObjectPtr<AActor> OwnerBoss;
public:
    UFUNCTION()
    void NotifyHitByPlayerWeapon(const FHitResult& Hit, AActor* Attacker);

    bool bBroken = false;
};
