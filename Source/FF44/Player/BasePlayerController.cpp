#include "Player/BasePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GenericTeamAgentInterface.h"
#include "InputMappingContext.h"
#include "AbilitySystemComponent.h"
#include "BasePlayerAttributeSet.h"

#include "UI/BasePlayerHUDWidget.h"
#include "UI/BaseMonsterHUDWidget.h"
#include "InventorySystem/Widget/InventoryWidget.h"


ABasePlayerController::ABasePlayerController()
{
	TeamId = FGenericTeamId(0);
}


void ABasePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* CurrentContext : InputMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}
	}
}

void ABasePlayerController::OnUnPossess()
{
	if (PlayerHUD->IsInViewport()) {
		PlayerHUD->RemoveFromParent();
	}


	if (InventoryWidget->IsInViewport()) {
		InventoryWidget->RemoveFromParent();
	}
}

void ABasePlayerController::InitPlayerUI(UAbilitySystemComponent* _AbilitySystem)
{
	if (!_AbilitySystem) return;

	// HUD
	if (PlayerHUDClass) {
		PlayerHUD = CreateWidget<UBasePlayerHUDWidget>(GetWorld(), PlayerHUDClass);
		PlayerHUD->SetOwningPlayer(this);
		PlayerHUD->AddToViewport();
		PlayerHUD->InitASC(_AbilitySystem, _AbilitySystem->GetSet<UAttributeSet>());
	}

	// Inventory Set
	if (InventoryWidgetClass) {
		InventoryWidget = CreateWidget<UInventoryWidget>(GetWorld(), InventoryWidgetClass);
		InventoryWidget->SetOwningPlayer(this);
		InventoryWidget->AddToViewport();
		InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ABasePlayerController::InitBossUI(UAbilitySystemComponent* _AbilitySystem)
{
	if (!_AbilitySystem) return;

	if (BossHUD)
	{
		if (!BossHUD->IsVisible())
		{
			BossHUD->Destruct();
			BossHUD = nullptr;
		}
		return;
	}

	if (BossHUDClass) {
		BossHUD = CreateWidget<UBaseMonsterHUDWidget>(GetWorld(), BossHUDClass);
		BossHUD->SetOwningPlayer(this);
		BossHUD->AddToViewport();
		BossHUD->InitASC(_AbilitySystem, _AbilitySystem->GetSet<UAttributeSet>());
	}
}

void ABasePlayerController::ToggleHUD()
{
	if (!PlayerHUD) return;

	if (PlayerHUD->GetVisibility() == ESlateVisibility::Collapsed)
		PlayerHUD->SetVisibility(ESlateVisibility::Visible);
	else
		PlayerHUD->SetVisibility(ESlateVisibility::Collapsed);
}

void ABasePlayerController::ToggleInventory()
{
	if (!InventoryWidget) return;

	UE_LOG(LogTemp, Log, TEXT("Inventory Toggle"));

	if (InventoryWidget->GetVisibility() == ESlateVisibility::Collapsed)
	{
		InventoryWidget->SetVisibility(ESlateVisibility::Visible);
		SetInputMode(FInputModeGameAndUI());
		SetShowMouseCursor(true);
	}
	else
	{
		InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
		SetShowMouseCursor(false);
		SetInputMode(FInputModeGameOnly());
	}
}
