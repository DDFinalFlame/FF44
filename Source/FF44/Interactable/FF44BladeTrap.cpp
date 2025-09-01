// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/FF44BladeTrap.h"
#include "Components/StaticMeshComponent.h"

AFF44BladeTrap::AFF44BladeTrap()
{
    PrimaryActorTick.bCanEverTick = true;

    Pivot = CreateDefaultSubobject<USceneComponent>(TEXT("Pivot"));
    RootComponent = Pivot;

    BladeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BladeMesh"));
    BladeMesh->SetupAttachment(Pivot);
    BladeMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    BladeMesh->SetGenerateOverlapEvents(true);
}

void AFF44BladeTrap::BeginPlay()
{
    Super::BeginPlay();
}

void AFF44BladeTrap::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    RunningTime += DeltaSeconds * SwingSpeed;
    float Angle = FMath::Sin(RunningTime) * SwingAmplitude;

    Pivot->SetRelativeRotation(FRotator(Angle, 0.f, 0.f));
}