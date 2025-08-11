// Fill out your copyright notice in the Description page of Project Settings.


#include "RoomBase.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"


ARoomBase::ARoomBase()
{
	PrimaryActorTick.bCanEverTick = true;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	DefaultSceneRoot->SetupAttachment(RootComponent);

	GeometryFolder = CreateDefaultSubobject<USceneComponent>(TEXT("GeometryFolder"));
	GeometryFolder->SetupAttachment(DefaultSceneRoot);

	OverlapFolder = CreateDefaultSubobject<USceneComponent>(TEXT("OverlapFolder"));
	OverlapFolder->SetupAttachment(DefaultSceneRoot);

	Floor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Floor"));
	Floor->SetupAttachment(GeometryFolder);

	Wall_1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wall_1"));
	Wall_1->SetupAttachment(GeometryFolder);
	Wall_2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wall_2"));
	Wall_2->SetupAttachment(GeometryFolder);
	Wall_3 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wall_3"));
	Wall_3->SetupAttachment(GeometryFolder);
	Wall_4 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wall_4"));
	Wall_4->SetupAttachment(GeometryFolder);
	Wall_5 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wall_5"));
	Wall_5->SetupAttachment(GeometryFolder);
	Wall_6 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wall_6"));
	Wall_6->SetupAttachment(GeometryFolder);
	Wall_7 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wall_7"));
	Wall_7->SetupAttachment(GeometryFolder);
	Wall_8 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wall_8"));
	Wall_8->SetupAttachment(GeometryFolder);

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Box_Collision"));
	BoxCollision->SetupAttachment(OverlapFolder);

	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	Arrow->SetupAttachment(DefaultSceneRoot);
	Arrow->bHiddenInGame = false;

}

void ARoomBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ARoomBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

