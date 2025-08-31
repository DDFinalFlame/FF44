
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "FallingRockActor.generated.h"
#include "GeometryCollection/GeometryCollectionComponent.h"

class UStaticMeshComponent;
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
    /** ������ ���� �޽�(��Ʈ) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* Mesh;

    // ������ �浹 �ڽ� (�޽ÿ� ������)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UBoxComponent* HitBox;

    UPROPERTY(VisibleAnywhere)
    UGeometryCollectionComponent* GeoComp;

    UPROPERTY(EditAnywhere, Category = "Rock|Asset")
    TSoftObjectPtr<UGeometryCollection> RockGC;

public:
    /** ���� �ϰ� �ӵ�(�߷¸����� ����ϸ� 0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock")
    float InitialDownSpeed = 0.f;

    /** ����(���ŷ)�� �浹 �� �ı����� */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock")
    bool bDestroyOnGroundHit = true;

    /** ���� �浹 �� �ı� ����(��) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock")
    float DestroyDelayOnGround = 0.4f;

    /** �ڵ� ����(0 �̸� ��Ȱ��) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock")
    float LifeSeconds = 6.f;

    /** ���� �浹 ����Ʈ(����) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock|FX")
    UNiagaraSystem* ImpactFX = nullptr;

    /** ���� �浹 ����(����) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock|FX")
    USoundBase* ImpactSound = nullptr;

    // === ������ / GAS ===
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rock|Damage")
    TSubclassOf<UGameplayEffect> GE_Damage; // �������� GE

    // ByCaller �±� (��: "Data.Damage")
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rock|Damage", meta = (Categories = "Data"))
    FGameplayTag ByCallerDamageTag;

    // ByCaller�� �ƴ� ���� ��ġ�ε� ��� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock|Damage")
    float DamageMagnitude = 0.f;

    // �ߺ�Ÿ�� ���� (�� ���ʹ� 1ȸ��)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock|Damage")
    bool bOncePerActor = true;

    // �ڱ���/���� �� ���� ��Ģ ��
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock|Damage")
    bool bIgnoreOwnerAndInstigatorTeam = true;

    // ������ ���� (GE ���ؽ�Ʈ�� �־���)
    UFUNCTION(BlueprintCallable)
    void SetDamageInstigator(AActor* InInstigator);

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, FVector NormalImpulse,
        const FHitResult& Hit);

    UFUNCTION()
    void OnHitBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    void ApplyDamageTo(AActor* Target, const FHitResult& OptionalHit);
    UAbilitySystemComponent* GetASCFromActor(AActor* Actor) const;

    bool ShouldIgnore(AActor* OtherActor) const;

private:
    TSet<TWeakObjectPtr<AActor>> AlreadyHitSet;
    TWeakObjectPtr<AActor> DamageInstigator;
};