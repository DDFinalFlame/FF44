// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator/DungeonBase/FF44PortalRoom.h"

AFF44PortalRoom::AFF44PortalRoom()
{
	PortalPoints = CreateDefaultSubobject<USceneComponent>(TEXT("PortalPoints"));
	PortalPoints->SetupAttachment(RootComponent);

	RoomTypeTag = "PortalRoom";
}
