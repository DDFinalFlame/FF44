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

    // ���� ���� �� �ı� �� ���� ���� ����
    void InitializeWeakPoint(AActor* _BossActor, float _DamageToBoss);

protected:
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;

    // �ı� �� ���� ���ط�
    float DamageToBossOnBreak = 10.f;

    // ���� ���۷���
    TWeakObjectPtr<AActor> OwnerBoss;

    UFUNCTION()
    void OnMeshHit(UPrimitiveComponent* _HitComp, AActor* _OtherActor,
        UPrimitiveComponent* _OtherComp, FVector _NormalImpulse, const FHitResult& _Hit);
public:
    // ���� �ı� ó�� (�ѹ���)
    void BreakAndNotify();

    bool bBroken = false;
};
