// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator/DungeonBase/CoinBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/RotatingMovementComponent.h"

ACoinBase::ACoinBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Coin = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Coin"));
	Coin->SetupAttachment(RootComponent);

	CoinCollision = CreateDefaultSubobject<USphereComponent>(TEXT("CoinCollision"));
	CoinCollision->SetupAttachment(Coin);

	RotatingMovementComponent = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovementComponent"));

}

void ACoinBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACoinBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

