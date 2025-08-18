// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator/DungeonBase/ClosingWall.h"

AClosingWall::AClosingWall()
{
	PrimaryActorTick.bCanEverTick = true;

	ClosingWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ClosingWall"));
	ClosingWall->SetupAttachment(RootComponent);

}

void AClosingWall::BeginPlay()
{
	Super::BeginPlay();
	
}

void AClosingWall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

