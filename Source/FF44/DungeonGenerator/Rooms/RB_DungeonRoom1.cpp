// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator/Rooms/RB_DungeonRoom1.h"
#include "Components/ArrowComponent.h"


ARB_DungeonRoom1::ARB_DungeonRoom1()
{
	Exit_Arrow_1 = CreateDefaultSubobject<UArrowComponent>(TEXT("Exit_Arrow_1"));
	Exit_Arrow_1->SetupAttachment(ExitPointsFolder);
	Exit_Arrow_2 = CreateDefaultSubobject<UArrowComponent>(TEXT("Exit_Arrow_2"));
	Exit_Arrow_2->SetupAttachment(ExitPointsFolder);
	Exit_Arrow_3 = CreateDefaultSubobject<UArrowComponent>(TEXT("Exit_Arrow_3"));
	Exit_Arrow_3->SetupAttachment(ExitPointsFolder);

	Spawn_Point_1 = CreateDefaultSubobject<UArrowComponent>(TEXT("Spawn_Point_1"));
	Spawn_Point_1->SetupAttachment(FloorSpawnPoints);
	Spawn_Point_2 = CreateDefaultSubobject<UArrowComponent>(TEXT("Spawn_Point_2"));
	Spawn_Point_2->SetupAttachment(FloorSpawnPoints);
	Spawn_Point_3 = CreateDefaultSubobject<UArrowComponent>(TEXT("Spawn_Point_3"));
	Spawn_Point_3->SetupAttachment(FloorSpawnPoints);
	Spawn_Point_4 = CreateDefaultSubobject<UArrowComponent>(TEXT("Spawn_Point_4"));
	Spawn_Point_4->SetupAttachment(FloorSpawnPoints);
	Spawn_Point_5 = CreateDefaultSubobject<UArrowComponent>(TEXT("Spawn_Point_5"));
	Spawn_Point_5->SetupAttachment(FloorSpawnPoints);
	Spawn_Point_6 = CreateDefaultSubobject<UArrowComponent>(TEXT("Spawn_Point_6"));
	Spawn_Point_6->SetupAttachment(FloorSpawnPoints);
	
}
