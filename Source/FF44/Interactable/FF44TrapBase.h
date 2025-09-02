// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable/FF44InteractableActor.h"
#include "FF44TrapBase.generated.h"

class UBoxComponent;
class UGameplayEffect;

UCLASS()
class FF44_API AFF44TrapBase : public AFF44InteractableActor
{
    GENERATED_BODY()

public:
    AFF44TrapBase();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayEffect")
    TSubclassOf<UGameplayEffect> GameplayEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trap|Trigger")
    UBoxComponent* TriggerBox;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Trigger")
    bool bUseTriggerBox = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
    bool bArmed = true;

    UPROPERTY(BlueprintReadOnly, Category = "Trap")
    bool bActive = false;

protected:
    UFUNCTION()
    void OnTriggerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& Sweep);

    UFUNCTION()
    void OnTriggerExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION(BlueprintCallable, Category = "Trap")
    virtual void SetActive(bool bInActive);

    UFUNCTION(BlueprintCallable, Category = "Trap")
    void SetArmed(bool bInArmed);

public:
    virtual bool CanInteract_Implementation(AActor* Interactor) const override;
    virtual void Interact_Implementation(AActor* Interactor) override;

    UFUNCTION(BlueprintImplementableEvent, Category = "Trap")
    void BP_OnTriggered();

    UFUNCTION(BlueprintImplementableEvent, Category = "Trap")
    void BP_OnArmed();

    UFUNCTION(BlueprintImplementableEvent, Category = "Trap")
    void BP_OnDisarmed();

    UFUNCTION(BlueprintImplementableEvent, Category = "Trap")
    void BP_OnActivate();

    UFUNCTION(BlueprintImplementableEvent, Category = "Trap")
    void BP_OnDeactivate();
};