#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BossLightningProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class MONSTERAIPLUGIN_API ABossLightningProjectile : public AActor
{
    GENERATED_BODY()

public:
    ABossLightningProjectile();

    virtual void BeginPlay() override;   
    virtual void Tick(float DeltaSeconds) override; 

    void InitProjectile(AActor* InTargetBoss, float InDamage);

protected:
    UPROPERTY(VisibleAnywhere)
    USphereComponent* Collision;

    UPROPERTY(VisibleAnywhere)
    UProjectileMovementComponent* Movement;

    TWeakObjectPtr<AActor> TargetBoss;
    float DamageValue = 0.f;

    UFUNCTION()
    void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    // ������
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDebugDraw = true;

    UPROPERTY(EditAnywhere, Category = "Debug")
    float DebugLineThickness = 1.5f;

    UPROPERTY(EditAnywhere, Category = "Debug")
    float DebugStayTime = 0.f; // �� ƽ ������ �Ŷ� 0�̸� ���

protected:
    UPROPERTY(EditAnywhere, Category = "Audio")
    USoundBase* SpawnSound = nullptr;   // ������ �� ���

    UPROPERTY(EditAnywhere, Category = "Audio")
    USoundBase* HitSound = nullptr;     // ������ ���� �� ���
};