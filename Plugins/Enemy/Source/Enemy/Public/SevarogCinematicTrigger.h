// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Boss/BossCinematicTrigger.h"
#include "SevarogCinematicTrigger.generated.h"

/**
 * 
 */
UCLASS()
class ENEMY_API ASevarogCinematicTrigger : public ABossCinematicTrigger
{
	GENERATED_BODY()

protected:
    virtual void TriggerOnce(AActor* _OtherActor) override;
};
