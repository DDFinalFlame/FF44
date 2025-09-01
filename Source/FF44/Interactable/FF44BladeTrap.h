// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable/FF44TrapBase.h"
#include "FF44BladeTrap.generated.h"

UCLASS()
class FF44_API AFF44BladeTrap : public AFF44TrapBase
{
    GENERATED_BODY()

public:
    AFF44BladeTrap();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BladeTrap")
    float SwingAmplitude = 60.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BladeTrap")
    float SwingSpeed = 1.5f;

    UPROPERTY(VisibleAnywhere, Category = "BladeTrap")
    USceneComponent* Pivot;

    UPROPERTY(VisibleAnywhere, Category = "BladeTrap")
    UStaticMeshComponent* BladeMesh;

private:
    float RunningTime = 0.f;
};