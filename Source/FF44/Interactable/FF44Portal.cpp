// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/FF44Portal.h"

AFF44Portal::AFF44Portal()
{
    PrimaryActorTick.bCanEverTick = true;
}

FName AFF44Portal::ResolvePortalTag() const
{
    if (Tags.Contains(TEXT("PortalBoss"))) return TEXT("PortalBoss");
    if (Tags.Contains(TEXT("PortalNext"))) return TEXT("PortalNext");
    return NAME_None;
}

void AFF44Portal::Interact_Implementation(AActor* Interactor)
{
    Super::Interact_Implementation(Interactor);

    const FName Tag = ResolvePortalTag();
    OnPortalInteracted.Broadcast(this, Tag);
}