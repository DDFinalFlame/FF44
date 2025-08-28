// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/FF44InteractableActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

AFF44InteractableActor::AFF44InteractableActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
    Mesh->SetCollisionProfileName(TEXT("BlockAll"));

    InteractRange = CreateDefaultSubobject<USphereComponent>(TEXT("InteractRange"));
    InteractRange->SetupAttachment(RootComponent);
    InteractRange->InitSphereRadius(150.f);
    InteractRange->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractRange->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractRange->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AFF44InteractableActor::BeginPlay()
{
	Super::BeginPlay();
}

bool AFF44InteractableActor::CanInteract_Implementation(AActor* Interactor) const
{
    return true;
}

void AFF44InteractableActor::Interact_Implementation(AActor* Interactor)
{
}
