// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/FF44TrapBase.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "Player/BasePlayer.h"
#include "GameplayEffect.h"

AFF44TrapBase::AFF44TrapBase()
{
    PrimaryActorTick.bCanEverTick = true;

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    TriggerBox->SetupAttachment(RootComponent);
    TriggerBox->SetBoxExtent(FVector(200.f, 200.f, 150.f));
    TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    TriggerBox->SetGenerateOverlapEvents(true);
}

void AFF44TrapBase::BeginPlay()
{
    Super::BeginPlay();

    if (!bUseTriggerBox && TriggerBox)
    {
        TriggerBox->SetGenerateOverlapEvents(false);
        TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        TriggerBox->SetHiddenInGame(true);
    }
    else if (bUseTriggerBox && TriggerBox)
    {
        TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AFF44TrapBase::OnTriggerEnter);
        TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AFF44TrapBase::OnTriggerExit);
    }

    bActive = false;
}

void AFF44TrapBase::OnTriggerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& Sweep)
{
    if (!bUseTriggerBox || !bArmed || !OtherActor) return;

    if (Cast<ABasePlayer>(OtherActor) || Cast<APawn>(OtherActor))
    {
        BP_OnTriggered();
        SetActive(true);
    }
}

void AFF44TrapBase::OnTriggerExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

bool AFF44TrapBase::CanInteract_Implementation(AActor* /*Interactor*/) const
{
    return bArmed;
}

void AFF44TrapBase::Interact_Implementation(AActor* /*Interactor*/)
{
    if (!bArmed) return;

    SetArmed(false);
    SetActive(false);
    SetPromptVisible(false);

    if (bUseTriggerBox && TriggerBox)
    {
        TriggerBox->SetGenerateOverlapEvents(false);
        TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void AFF44TrapBase::SetActive(bool bInActive)
{
    if (bActive == bInActive) return;
    bActive = bInActive;

    if (bActive) BP_OnActivate();
    else         BP_OnDeactivate();
}

void AFF44TrapBase::SetArmed(bool bInArmed)
{
    if (bArmed == bInArmed) return;
    bArmed = bInArmed;

    if (bArmed) BP_OnArmed();
    else        BP_OnDisarmed();
}

void AFF44TrapBase::OnDamageBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{

}

void AFF44TrapBase::OnDamageEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}
