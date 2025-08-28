
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FallingRockActor.generated.h"

class UStaticMeshComponent;
class UNiagaraSystem;
class USoundBase;

UCLASS()
class MONSTERAIPLUGIN_API AFallingRockActor : public AActor
{
    GENERATED_BODY()

public:
    AFallingRockActor();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, FVector NormalImpulse,
        const FHitResult& Hit);

    UFUNCTION()
    void OnMeshBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

protected:
    /** 떨어질 바위 메시(루트) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* Mesh;

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
};