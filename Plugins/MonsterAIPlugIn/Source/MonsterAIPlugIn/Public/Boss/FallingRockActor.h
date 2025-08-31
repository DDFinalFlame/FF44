
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
    /** 떨어질 바위 메시(루트) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* Mesh;

    // 판정용 충돌 박스 (메시와 별도로)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UBoxComponent* HitBox;

    UPROPERTY(VisibleAnywhere)
    UGeometryCollectionComponent* GeoComp;

    UPROPERTY(EditAnywhere, Category = "Rock|Asset")
    TSoftObjectPtr<UGeometryCollection> RockGC;

public:
    /** 시작 하강 속도(중력만으로 충분하면 0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock")
    float InitialDownSpeed = 0.f;

    /** 지면(블로킹)과 충돌 시 파괴할지 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock")
    bool bDestroyOnGroundHit = true;

    /** 지면 충돌 후 파괴 지연(초) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock")
    float DestroyDelayOnGround = 0.4f;

    /** 자동 수명(0 이면 비활성) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock")
    float LifeSeconds = 6.f;

    /** 지면 충돌 이펙트(선택) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock|FX")
    UNiagaraSystem* ImpactFX = nullptr;

    /** 지면 충돌 사운드(선택) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock|FX")
    USoundBase* ImpactSound = nullptr;

    // === 데미지 / GAS ===
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rock|Damage")
    TSubclassOf<UGameplayEffect> GE_Damage; // 데미지용 GE

    // ByCaller 태그 (예: "Data.Damage")
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rock|Damage", meta = (Categories = "Data"))
    FGameplayTag ByCallerDamageTag;

    // ByCaller가 아닌 고정 수치로도 사용 가능
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock|Damage")
    float DamageMagnitude = 0.f;

    // 중복타격 방지 (한 액터당 1회만)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock|Damage")
    bool bOncePerActor = true;

    // 자기편/보스 등 무시 규칙 훅
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock|Damage")
    bool bIgnoreOwnerAndInstigatorTeam = true;

    // 가해자 정보 (GE 컨텍스트에 넣어줌)
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