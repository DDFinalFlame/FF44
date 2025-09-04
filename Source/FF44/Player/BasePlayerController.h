// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "BasePlayerController.generated.h"

class UInputMappingContext;
class UAbilitySystemComponent;
class UBasePlayerAttributeSet;

class UBasePlayerHUDWidget;

UCLASS()
class FF44_API ABasePlayerController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
public:
	ABasePlayerController();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InputMapping")
	TArray<UInputMappingContext*> InputMappingContexts;

	virtual void SetupInputComponent() override;

///////////////////////////////////////////////////////////////////////////////////////
///										UI											///
///////////////////////////////////////////////////////////////////////////////////////

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UBasePlayerHUDWidget> PlayerHUDClass;

public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void InitUI(UAbilitySystemComponent* _AbilitySystem);

	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }
protected:
	FGenericTeamId TeamId;
};
