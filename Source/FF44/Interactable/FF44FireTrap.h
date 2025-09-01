// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable/FF44TrapBase.h"
#include "FF44FireTrap.generated.h"

class UBoxComponent;
class UParticleSystemComponent;

UCLASS()
class FF44_API AFF44FireTrap : public AFF44TrapBase
{
    GENERATED_BODY()

public:
    AFF44FireTrap();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trap|Components")
    UBoxComponent* DamageArea;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trap|Components")
    USceneComponent* FXRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trap|Components")
    UParticleSystemComponent* FireEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Cycle")
    float ActiveTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Cycle")
    float InactiveTime = 2.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Trap|Cycle")
    bool bActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Damage")
    float PerActorDamageCooldown = 0.5f;

private:
    FTimerHandle CycleTimerHandle;

    TMap<TWeakObjectPtr<AActor>, float> LastHitTime;

    void StartCycle();
    void SetActive(bool bInActive);
    void OnCycleTick();

    // Overlap
    UFUNCTION()
    void OnDamageAreaBegin(UPrimitiveComponent* Overlapped, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIdx, bool bFromSweep,
        const FHitResult& Sweep);

    UFUNCTION()
    void OnDamageAreaEnd(UPrimitiveComponent* Overlapped, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIdx);

    virtual void Interact_Implementation(AActor* Interactor) override;
};