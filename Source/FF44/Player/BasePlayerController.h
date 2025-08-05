// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BasePlayerController.generated.h"

class UInputMappingContext;

UCLASS()
class FF44_API ABasePlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InputMapping")
	TArray<UInputMappingContext*> InputMappingContexts;

	virtual void SetupInputComponent() override;
};
