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
class UBaseMonsterHUDWidget;
class UInventoryWidget;

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
	virtual void OnUnPossess() override;

///////////////////////////////////////////////////////////////////////////////////////
///										UI											///
///////////////////////////////////////////////////////////////////////////////////////

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UBasePlayerHUDWidget> PlayerHUDClass;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<UBasePlayerHUDWidget> PlayerHUD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UBaseMonsterHUDWidget> BossHUDClass;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<UBaseMonsterHUDWidget> BossHUD;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UInventoryWidget> InventoryWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<UInventoryWidget> InventoryWidget;

public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void InitPlayerUI(UAbilitySystemComponent* _AbilitySystem);
	void InitBossUI(UAbilitySystemComponent* _AbilitySystem);

	virtual void ToggleHUD();
	virtual void ToggleInventory();

	UFUNCTION()
	virtual void OpenInventory();

	UFUNCTION()
	virtual void CloseInventory();

	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }
	virtual UInventoryWidget* GetInventoryWidget() const { return InventoryWidget; }
	virtual UBasePlayerHUDWidget* GetHUDWIdget() const { return PlayerHUD; }
protected:
	FGenericTeamId TeamId;
};
