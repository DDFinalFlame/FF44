// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable/FF44TrapBase.h"
#include "FF44FireTrap.generated.h"

class UParticleSystemComponent;
class UBoxComponent;
class UDamageType;

UCLASS()
class FF44_API AFF44FireTrap : public AFF44TrapBase
{
    GENERATED_BODY()

public:
    AFF44FireTrap();

protected:
    virtual void BeginPlay() override;

    virtual void SetActive(bool bInActive) override;

    UFUNCTION()
    void OnDamageAreaBegin(UPrimitiveComponent* Overlapped, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIdx, bool bFromSweep, const FHitResult& Sweep);

    UFUNCTION()
    void OnDamageAreaEnd(UPrimitiveComponent* Overlapped, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIdx);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trap|Fire")
    UBoxComponent* DamageArea;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trap|Fire")
    UParticleSystemComponent* FireEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Fire")
    float Damage = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Fire")
    TSubclassOf<UDamageType> DamageTypeClass;
};