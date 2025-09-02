#include "Player/BasePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GenericTeamAgentInterface.h"
#include "InputMappingContext.h"
#include "AbilitySystemComponent.h"
#include "BasePlayerAttributeSet.h"

#include "UI/BasePlayerHUDWidget.h"


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

void ABasePlayerController::InitPlayerUI(UAbilitySystemComponent* _AbilitySystem)
{
	if (!_AbilitySystem) return;

	// HUD
	if (PlayerHUDClass) {
		PlayerHUD = CreateWidget<UBasePlayerHUDWidget>(GetWorld(), PlayerHUDClass);
		PlayerHUD->SetOwningPlayer(this);
		PlayerHUD->InitASC(_AbilitySystem, _AbilitySystem->GetSet<UBasePlayerAttributeSet>());
	}

	// Inventory Set
	if (InventoryWidgetClass) {
		InventoryWidget = CreateWidget<UUserWidget>(GetWorld(), InventoryWidgetClass);
		InventoryWidget->SetOwningPlayer(this);
	}
}

void ABasePlayerController::ToggleHUD()
{
	if (!PlayerHUD) return;

	if (PlayerHUD->IsInViewport()) {
		PlayerHUD->RemoveFromParent();
	}
	else {
		PlayerHUD->AddToViewport();
	}
}

void ABasePlayerController::ToggleInventory()
{
	if (!InventoryWidget) return;

	UE_LOG(LogTemp, Warning, TEXT("Inventory Toggle"));

	if (InventoryWidget->IsInViewport()) {
		InventoryWidget->RemoveFromParent();
		SetShowMouseCursor(false);
		SetInputMode(FInputModeGameOnly());
	}
	else {
		InventoryWidget->AddToViewport();		
		SetShowMouseCursor(true);
		SetInputMode(FInputModeGameAndUI());
	}
}
