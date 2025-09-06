// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable/FF44InteractableActor.h"
#include "FF44Portal.generated.h"

class AFF44Portal;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPortalInteracted, AFF44Portal*, Portal, FName, PortalTag);

UCLASS()
class FF44_API AFF44Portal : public AFF44InteractableActor
{
    GENERATED_BODY()

public:
    AFF44Portal();

    virtual void Interact_Implementation(AActor* Interactor) override;

public:
    UPROPERTY(BlueprintAssignable, Category = "Portal|Events")
    FOnPortalInteracted OnPortalInteracted;

private:
    FName ResolvePortalTag() const;
};