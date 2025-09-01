
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Chaos/ChaosEngineInterface.h"           // FChaosPhysicsCollisionInfo
#include "Field/FieldSystemComponent.h"
#include "Field/FieldSystemObjects.h"
#include "FallingRockActor.generated.h"


class UBoxComponent;
class UNiagaraSystem;
class USoundBase;
class UGameplayEffect;
class UAbilitySystemComponent;

UCLASS()
class MONSTERAIPLUGIN_API AFallingRockActor : public AActor
{
    GENERATED_BODY()

public:
    AFallingRockActor();

protected:
    /** ����ó��: GeometryCollection ��Ʈ */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UGeometryCollectionComponent* GeoComp;

    /** ������ �浹 �ڽ� (�޽ÿ� ������) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UBoxComponent* HitBox;

    /** (����) �ʵ� ����� FieldSystem ������Ʈ */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UFieldSystemComponent* FieldSystem;

public:
    // ====== ���� ���� ������Ƽ ���� ======
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock")
    float InitialDownSpeed = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock")
    bool bDestroyOnGroundHit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock")
    float DestroyDelayOnGround = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock")
    float LifeSeconds = 6.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock|FX")
    UNiagaraSystem* ImpactFX = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock|FX")
    USoundBase* ImpactSound = nullptr;

    // === ������ / GAS ===
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rock|Damage")
    TSubclassOf<UGameplayEffect> GE_Damage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rock|Damage", meta = (Categories = "Data"))
    FGameplayTag ByCallerDamageTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock|Damage")
    float DamageMagnitude = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock|Damage")
    bool bOncePerActor = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock|Damage")
    bool bIgnoreOwnerAndInstigatorTeam = true;

    UFUNCTION(BlueprintCallable)
    void SetDamageInstigator(AActor* InInstigator);

    /** ���� �浹 �� ����ó ���� ������ ����� �ݰ�/���� */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock|Fracture")
    float FractureRadius = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock|Fracture")
    float FractureStrength = 50000.f;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds);

    /** Chaos ���� �浹 �̺�Ʈ(GeoComp) */
    UFUNCTION()
    void OnChaosCollision(const FChaosPhysicsCollisionInfo& CollisionInfo);

    /** GeometryCollection �극��ũ �̺�Ʈ(���� �и�) */
    UFUNCTION()
    void OnChaosBreak(const FChaosBreakEvent& BreakEvent);

    UFUNCTION()
    void OnHitBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    void ApplyDamageTo(AActor* Target, const FHitResult& OptionalHit);
    UAbilitySystemComponent* GetASCFromActor(AActor* Actor) const;

    bool ShouldIgnore(AActor* OtherActor) const;

    /** �ʵ�(��Ʈ����)�� ����ó�� �����ϴ� ���� */
    void ApplyFractureFieldAt(const FVector& Center);

private:
    TSet<TWeakObjectPtr<AActor>> AlreadyHitSet;
    TWeakObjectPtr<AActor> DamageInstigator;
    bool bHasLanded = false;
};