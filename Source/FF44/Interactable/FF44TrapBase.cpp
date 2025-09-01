// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/FF44TrapBase.h"

AFF44TrapBase::AFF44TrapBase()
{
    PrimaryActorTick.bCanEverTick = false;
}

bool AFF44TrapBase::CanInteract_Implementation(AActor* Interactor) const
{
    return bCanBeDisarmed && bArmed;
}

void AFF44TrapBase::Interact_Implementation(AActor* Interactor)
{
    if (!CanInteract_Implementation(Interactor)) return;

    SetArmed(false);
    SetPromptVisible(false);
}

void AFF44TrapBase::SetArmed(bool bInArmed)
{
    bArmed = bInArmed;
    if (bArmed)
    {
        BP_OnArmed();
    }
    else
    {
        BP_OnDisarmed();
    }
}
