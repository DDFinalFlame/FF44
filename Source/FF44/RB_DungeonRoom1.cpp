// Fill out your copyright notice in the Description page of Project Settings.


#include "RB_DungeonRoom1.h"
#include "Components/ArrowComponent.h"


ARB_DungeonRoom1::ARB_DungeonRoom1()
{
	Exit_Arrow_1 = CreateDefaultSubobject<UArrowComponent>(TEXT("Exit_Arrow_1"));
	Exit_Arrow_1->SetupAttachment(ExitPointsFolder);
	Exit_Arrow_2 = CreateDefaultSubobject<UArrowComponent>(TEXT("Exit_Arrow_2"));
	Exit_Arrow_2->SetupAttachment(ExitPointsFolder);
	Exit_Arrow_3 = CreateDefaultSubobject<UArrowComponent>(TEXT("Exit_Arrow_3"));
	Exit_Arrow_3->SetupAttachment(ExitPointsFolder);

}
