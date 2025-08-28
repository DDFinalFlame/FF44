// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator/DungeonBase/FF44StarterRoom.h"
#include "Components/ArrowComponent.h"

AFF44StarterRoom::AFF44StarterRoom()
{
	PrimaryActorTick.bCanEverTick = false;

	PlayerStart = CreateDefaultSubobject<USceneComponent>(TEXT("PlayerStart"));
	PlayerStart->SetupAttachment(RootComponent);

	PlayerStartPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("PlayerStartPoint"));
	PlayerStartPoint->SetupAttachment(PlayerStart);
    PlayerStartPoint->ArrowSize = 1.5f;

}

bool AFF44StarterRoom::GetPlayerStartTransform(FTransform& OutTransform) const
{
    if (PlayerStartPoint)
    {
        OutTransform = PlayerStartPoint->GetComponentTransform();
        return true;
    }

    OutTransform = FTransform::Identity;
    return false;
}