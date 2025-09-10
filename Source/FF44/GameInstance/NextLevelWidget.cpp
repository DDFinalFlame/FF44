#include "GameInstance/NextLevelWidget.h"
#include "Player/BasePlayer.h"
#include "GameInstance/FF44GameInstance.h"

void UNextLevelWidget::DataToss()
{
	if (auto PC = GetOwningPlayer())
		if (auto owner = PC->GetPawn())
			if (auto player = Cast<ABasePlayer>(owner))
				if (auto inst = GetWorld()->GetGameInstance())
					if (auto instance = Cast<UFF44GameInstance>(inst))
						instance->PendingCompState.CaptureFrom(player->GetAbilitySystemComponent(), player->GetInventoryComponent());

	// Instance에서 Character를 BeginPlay 할 때마다 들게끔 하는방법?
}
