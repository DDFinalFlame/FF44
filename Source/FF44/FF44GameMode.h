// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FF44GameMode.generated.h"

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class AFF44GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	AFF44GameMode();
};



