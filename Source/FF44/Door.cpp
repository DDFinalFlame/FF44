// Fill out your copyright notice in the Description page of Project Settings.


#include "Door.h"
#include "Components/BoxComponent.h"


ADoor::ADoor()
{
	PrimaryActorTick.bCanEverTick = true;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Default_Scene_Root"));
	DefaultSceneRoot->SetupAttachment(RootComponent);

	Door = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
	Door->SetupAttachment(DefaultSceneRoot);

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Box_Collision"));
	BoxCollision->SetupAttachment(DefaultSceneRoot);

}

void ADoor::BeginPlay()
{
	Super::BeginPlay();
	
	StartLocation = Door->GetRelativeLocation();

}

void ADoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentLocation = Door->GetRelativeLocation();

	if (bSouldMove)
	{
		FVector NewLocation = FMath::VInterpTo(CurrentLocation, EndLocation, DeltaTime, MoveSpeed);
		Door->SetRelativeLocation(NewLocation);
	}

}

void ADoor::OpenDoor()
{
	EndLocation = CurrentLocation - FVector(0.0f, 0.0f, 400.0f);
	bSouldMove = true;
}

void ADoor::CloseDoor()
{
	EndLocation = StartLocation;
	bSouldMove = true;
}

