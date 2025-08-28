
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
    /** ������ ���� �޽�(��Ʈ) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* Mesh;

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
};