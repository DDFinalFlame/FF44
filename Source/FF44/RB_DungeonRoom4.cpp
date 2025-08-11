// Fill out your copyright notice in the Description page of Project Settings.


#include "RB_DungeonRoom4.h"
#include "Components/ArrowComponent.h"


ARB_DungeonRoom4::ARB_DungeonRoom4()
{
	Start_Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Start_Arrow"));
	Start_Arrow->SetupAttachment(ExitPointsFolder);
	Exit_Arrow_1 = CreateDefaultSubobject<UArrowComponent>(TEXT("Exit_Arrow_1"));
	Exit_Arrow_1->SetupAttachment(ExitPointsFolder);
	ClosingWall_1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ClosingWall_1"));
	ClosingWall_1->SetupAttachment(GeometryFolder);
	ClosingWall_2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ClosingWall_2"));
	ClosingWall_2->SetupAttachment(GeometryFolder);

}
