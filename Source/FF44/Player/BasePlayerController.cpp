#include "Player/BasePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "AbilitySystemComponent.h"
#include "BasePlayerAttributeSet.h"

#include "UI/BasePlayerHUDWidget.h"

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

void ABasePlayerController::InitUI(UAbilitySystemComponent* _AbilitySystem)
{
	if (!PlayerHUDClass) return;
	auto HUD = CreateWidget<UBasePlayerHUDWidget>(GetWorld(), PlayerHUDClass);
	HUD->InitASC(_AbilitySystem, _AbilitySystem->GetSet<UBasePlayerAttributeSet>());

	HUD->AddToViewport();
}
