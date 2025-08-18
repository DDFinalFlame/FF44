// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator/DungeonBase/TreasureChestBase.h"

ATreasureChestBase::ATreasureChestBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Chest = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Chest"));
	Chest->SetupAttachment(RootComponent);

}

void ATreasureChestBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATreasureChestBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

