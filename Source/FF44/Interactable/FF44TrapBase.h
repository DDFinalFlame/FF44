// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable/FF44InteractableActor.h"
#include "FF44TrapBase.generated.h"

UCLASS()
class FF44_API AFF44TrapBase : public AFF44InteractableActor
{
    GENERATED_BODY()

public:
    AFF44TrapBase();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
    bool bArmed = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
    bool bCanBeDisarmed = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
    bool bReArmable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Damage")
    float Damage = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Damage")
    TSubclassOf<UDamageType> DamageTypeClass;

    UFUNCTION(BlueprintImplementableEvent, Category = "Trap")
    void BP_OnArmed();

    UFUNCTION(BlueprintImplementableEvent, Category = "Trap")
    void BP_OnDisarmed();

    // 인터랙션: 해체
    virtual bool CanInteract_Implementation(AActor* Interactor) const override;
    virtual void Interact_Implementation(AActor* Interactor) override;

    UFUNCTION(BlueprintCallable, Category = "Trap")
    void SetArmed(bool bInArmed);
};
