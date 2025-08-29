// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/FF44InteractableActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Interactable/FF44Interactable.h"

AFF44InteractableActor::AFF44InteractableActor()
{
    PrimaryActorTick.bCanEverTick = true;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
    Mesh->SetCollisionProfileName(TEXT("BlockAll"));

    InteractRange = CreateDefaultSubobject<USphereComponent>(TEXT("InteractRange"));
    InteractRange->SetupAttachment(RootComponent);
    InteractRange->InitSphereRadius(150.f);
    InteractRange->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractRange->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractRange->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    PromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PromptWidget"));
    PromptWidget->SetupAttachment(RootComponent);
    PromptWidget->SetWidgetSpace(EWidgetSpace::World);
    PromptWidget->SetDrawSize(FVector2D(100.f, 25.f));
    PromptWidget->SetTranslucentSortPriority(10);
    PromptWidget->SetTwoSided(true);
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

void AFF44InteractableActor::OnRangeBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
    if (!OtherActor || OtherActor == this) return;
    if (IFF44Interactable::Execute_CanInteract(this, OtherActor))
    {
        CurrentInteractor = OtherActor;
        SetPromptVisible(true);
        BP_OnFocus(OtherActor);
    }
}

void AFF44InteractableActor::OnRangeEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor || OtherActor == this) return;
    if (CurrentInteractor.Get() == OtherActor)
    {
        BP_OnUnfocus(OtherActor);
        SetPromptVisible(false);
        CurrentInteractor = nullptr;
    }
}

bool AFF44InteractableActor::CanInteract_Implementation(AActor* Interactor) const
{
    return true;
}

void AFF44InteractableActor::Interact_Implementation(AActor* Interactor)
{

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
    if (!PromptWidget || !PromptWidget->IsVisible())
        return;

    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC) return;

    APlayerCameraManager* Cam = PC->PlayerCameraManager;
    if (!Cam) return;

    const FVector Origin = GetActorLocation();

    const FVector CamLoc = Cam->GetCameraLocation();
    FVector ToCam = CamLoc - Origin;
    ToCam.Z = 0.f;
    const FVector Dir = ToCam.GetSafeNormal();

    const FVector TargetLoc = Origin + Dir * PromptDistance + FVector(0, 0, PromptHeight);
    PromptWidget->SetWorldLocation(TargetLoc);

    FRotator LookAt = (CamLoc - TargetLoc).Rotation();
    if (bYawOnly) { LookAt.Pitch = 0.f; LookAt.Roll = 0.f; }
    PromptWidget->SetWorldRotation(LookAt);
}
