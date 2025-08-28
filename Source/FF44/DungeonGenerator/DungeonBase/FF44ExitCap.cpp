// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator/DungeonBase/FF44ExitCap.h"

AFF44ExitCap::AFF44ExitCap()
{
	PrimaryActorTick.bCanEverTick = true;

	ExitCap = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ClosingWall"));
	ExitCap->SetupAttachment(RootComponent);
}

void AFF44ExitCap::BeginPlay()
{
	Super::BeginPlay();
	
}

void AFF44ExitCap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

