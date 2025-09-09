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

    // 보스 참조 및 파괴 시 가할 피해 설정
    void InitializeWeakPoint(AActor* _BossActor, float _DamageToBoss);

protected:
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;

    UFUNCTION()
    void OnMeshHit(UPrimitiveComponent* _HitComp, AActor* _OtherActor,
        UPrimitiveComponent* _OtherComp, FVector _NormalImpulse, const FHitResult& _Hit);

    UPROPERTY(EditAnywhere, Category = "WeakPoint|Timing")
    float BreakDelayMin = 0.5f;

    /** 맞은 뒤 사라질 때까지 최대 지연(초) */
    UPROPERTY(EditAnywhere, Category = "WeakPoint|Timing")
    float BreakDelayMax = 1.0f;

    /** 한 번만 트리거되도록 가드 */
    bool bPendingBreak = false;

    /** 지연 파괴 타이머 핸들 */
    FTimerHandle BreakTimerHandle;

    // 파괴 시 보낼 피해량
    float DamageToBossOnBreak = 10.f;
    // 무기에서 받은 히트 정보 캐싱(나이아가라/투사체 시작점으로 사용)
    FVector CachedImpactPoint = FVector::ZeroVector;
    TWeakObjectPtr<AActor> CachedAttacker;

    // === VFX / 투사체 옵션 ===
    UPROPERTY(EditAnywhere, Category = "WeakPoint|VFX")
    UNiagaraSystem* BreakVFX = nullptr;

    UPROPERTY(EditAnywhere, Category = "WeakPoint|VFX")
    FVector VFXOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "WeakPoint|Projectile")
    TSubclassOf<AActor> ProjectileClass; // 번개 투사체 액터 클래스

    UPROPERTY(EditAnywhere, Category = "WeakPoint|Projectile")
    float ProjectileSpeed = 2000.f;

    // (보스 쪽으로 날릴 때 회전/스폰 로직)
    void SpawnProjectileTowardBoss();
    // 보스 레퍼런스

    /** 지연 종료 시 호출(여기서 나이아가라/투사체도 이어서 가능) */
    void HandleDelayedBreak();

    // 파괴 처리
    void BreakAndNotify();
   
    TWeakObjectPtr<AActor> OwnerBoss;
public:
    UFUNCTION()
    void NotifyHitByPlayerWeapon(const FHitResult& Hit, AActor* Attacker);

    bool bBroken = false;
};
