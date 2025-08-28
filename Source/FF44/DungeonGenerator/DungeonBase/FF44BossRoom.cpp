// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator/DungeonBase/FF44BossRoom.h"
#include "FF44BossRoom.h"

AFF44BossRoom::AFF44BossRoom()
{
    PortalPoints = CreateDefaultSubobject<USceneComponent>(TEXT("PortalPoints"));
    PortalPoints->SetupAttachment(RootComponent);

    BossPoints = CreateDefaultSubobject<USceneComponent>(TEXT("BossPoints"));
    BossPoints->SetupAttachment(RootComponent);

    RoomTypeTag = "BossRoom";
}
