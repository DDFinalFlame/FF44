// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeakPointActor.generated.h"


class UStaticMeshComponent;

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

    // 파괴 시 보낼 피해량
    float DamageToBossOnBreak = 10.f;

    // 보스 레퍼런스
    TWeakObjectPtr<AActor> OwnerBoss;

    UFUNCTION()
    void OnMeshHit(UPrimitiveComponent* _HitComp, AActor* _OtherActor,
        UPrimitiveComponent* _OtherComp, FVector _NormalImpulse, const FHitResult& _Hit);
public:
    // 실제 파괴 처리 (한번만)
    void BreakAndNotify();

    bool bBroken = false;
};
