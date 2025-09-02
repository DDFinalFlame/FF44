// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable/FF44TrapBase.h"
#include "FF44SpikeTrap.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

UCLASS()
class FF44_API AFF44SpikeTrap : public AFF44TrapBase
{
    GENERATED_BODY()

public:
    AFF44SpikeTrap();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnTriggerBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& Sweep);

    UFUNCTION()
    void OnTriggerEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    virtual bool CanInteract_Implementation(AActor* Interactor) const override;
    virtual void Interact_Implementation(AActor* Interactor) override;

private:
    void StartCycle();
    void RaiseSpikes();
    void RetractSpikes();
    void EndCooldown();

    void ApplyDamageOnce();

private:
    UPROPERTY(VisibleAnywhere, Category = "SpikeTrap|Components")
    USceneComponent* SpikeRoot;

    UPROPERTY(VisibleAnywhere, Category = "SpikeTrap|Components")
    UStaticMeshComponent* BaseMesh;

    UPROPERTY(VisibleAnywhere, Category = "SpikeTrap|Components")
    UStaticMeshComponent* SpikeMesh;

    UPROPERTY(VisibleAnywhere, Category = "SpikeTrap|Components")
    UBoxComponent* DamageZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpikeTrap|Timing", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
    float TriggerDelay = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpikeTrap|Timing", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
    float HoldTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpikeTrap|Timing", meta = (AllowPrivateAccess = "true", ClampMin = "0"))
    float CooldownTime = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpikeTrap|Motion", meta = (AllowPrivateAccess = "true"))
    float SpikeRaiseDistance = 60.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpikeTrap|Motion", meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
    float RaiseTime = 0.20f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpikeTrap|Motion", meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
    float LowerTime = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpikeTrap|Motion", meta = (AllowPrivateAccess = "true"))
    bool bEaseIn = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpikeTrap|Motion", meta = (AllowPrivateAccess = "true"))
    bool bEaseOut = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpikeTrap|Damage", meta = (AllowPrivateAccess = "true"))
    float Damage = 25.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpikeTrap|Damage", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UDamageType> DamageType;

    bool bCycling = false;
    bool bDisarmed = false;

    FVector SpikeDownLocal;
    FVector SpikeUpLocal;

    int32 OverlapCount = 0;

    FTimerHandle DelayHandle;
    FTimerHandle HoldHandle;
    FTimerHandle CooldownHandle;

    UFUNCTION()
    void OnRaised();

    UFUNCTION()
    void OnRetracted();
};