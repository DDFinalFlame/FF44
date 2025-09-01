// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/FF44InteractableActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SceneComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Player/BasePlayer.h"

AFF44InteractableActor::AFF44InteractableActor()
{
    PrimaryActorTick.bCanEverTick = true;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
    Mesh->SetCollisionProfileName(TEXT("BlockAll"));

    InteractRange = CreateDefaultSubobject<USphereComponent>(TEXT("InteractRange"));
    InteractRange->SetupAttachment(RootComponent);
    InteractRange->InitSphereRadius(200.f);
    InteractRange->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractRange->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractRange->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    PromptAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("PromptAnchor"));
    PromptAnchor->SetupAttachment(RootComponent);

    PromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PromptWidget"));
    PromptWidget->SetupAttachment(PromptAnchor);
    PromptWidget->SetWidgetSpace(EWidgetSpace::World);
    PromptWidget->SetTwoSided(true);
    PromptWidget->SetDrawSize(FVector2D(120.f, 32.f));
    PromptWidget->SetVisibility(false);
}

void AFF44InteractableActor::BeginPlay()
{
    Super::BeginPlay();

    if (PromptWidget && PromptWidgetClass)
    {
        PromptWidget->SetWidgetClass(PromptWidgetClass);
    }

    if (InteractRange)
    {
        InteractRange->OnComponentBeginOverlap.AddDynamic(this, &AFF44InteractableActor::OnRangeBegin);
        InteractRange->OnComponentEndOverlap.AddDynamic(this, &AFF44InteractableActor::OnRangeEnd);
    }

    SetPromptVisible(false);
}

void AFF44InteractableActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    UpdatePromptFacing();
}

bool AFF44InteractableActor::CanInteract_Implementation(AActor* Interactor) const
{
    return true;
}

void AFF44InteractableActor::Interact_Implementation(AActor* Interactor)
{
    // 실제 상호작용 로직은 자식 클래스에서
}

void AFF44InteractableActor::OnFocus_Implementation(AActor* Interactor)
{
    CurrentInteractor = Interactor;
    SetPromptVisible(true);
}

void AFF44InteractableActor::OnUnfocus_Implementation(AActor* Interactor)
{
    if (CurrentInteractor.Get() == Interactor)
    {
        SetPromptVisible(false);
        CurrentInteractor = nullptr;
    }
}

void AFF44InteractableActor::SetPromptVisible(bool bVisible)
{
    if (PromptWidget)
    {
        PromptWidget->SetVisibility(bVisible);
    }
}

void AFF44InteractableActor::UpdatePromptFacing()
{
    if (!PromptWidget || !PromptWidget->IsVisible()) return;

    UWorld* World = GetWorld(); if (!World) return;
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0); if (!PC) return;

    FVector CamLoc; FRotator CamRot;
    PC->GetPlayerViewPoint(CamLoc, CamRot);

    const FVector AnchorLoc = PromptAnchor ? PromptAnchor->GetComponentLocation() : GetActorLocation();

    FVector TargetLoc = FMath::Lerp(AnchorLoc, CamLoc, FMath::Clamp(PromptMidAlpha, 0.f, 1.f));
    if (bKeepZFromAnchor) TargetLoc.Z = AnchorLoc.Z;

    PromptWidget->SetWorldLocation(TargetLoc);

    FRotator LookAt = (CamLoc - TargetLoc).Rotation();
    LookAt.Pitch = 0.f;
    LookAt.Roll = 0.f;
    PromptWidget->SetWorldRotation(LookAt);
}

void AFF44InteractableActor::OnRangeBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& Sweep)
{
    if (!OtherActor || OtherActor == this) return;

    if (ABasePlayer* Player = Cast<ABasePlayer>(OtherActor))
    {
        Player->NotifyInteractableInRange(this, /*bEnter=*/true);
    }
}

void AFF44InteractableActor::OnRangeEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor || OtherActor == this) return;

    if (ABasePlayer* Player = Cast<ABasePlayer>(OtherActor))
    {
        Player->NotifyInteractableInRange(this, /*bEnter=*/false);
    }
}
