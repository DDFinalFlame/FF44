// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable/FF44TrapBase.h"
#include "FF44BladeTrap.generated.h"

class UBoxComponent;
class USoundBase;

UCLASS()
class FF44_API AFF44BladeTrap : public AFF44TrapBase
{
    GENERATED_BODY()

public:
    AFF44BladeTrap();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    virtual void OnDamageBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep) override;

    virtual void OnDamageEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BladeTrap")
    float SwingAmplitude = 60.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BladeTrap")
    float SwingSpeed = 1.5f;

    UPROPERTY(VisibleAnywhere, Category = "BladeTrap")
    USceneComponent* Pivot;

    UPROPERTY(VisibleAnywhere, Category = "BladeTrap")
    UStaticMeshComponent* BladeMesh;

    UPROPERTY(VisibleAnywhere, Category = "BladeTrap")
    UBoxComponent* DamageArea;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BladeTrap|Audio", meta = (AllowPrivateAccess = "true"))
    USoundBase* MidPassSFX = nullptr;

private:
    float RunningTime = 0.f;

    FRotator BaseRotation;
    FVector  HingeAxisWorld;

    float PrevAngleDeg = 0.f;
    bool  bFirstTick = true;
};
