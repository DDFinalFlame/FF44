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

	// Instance���� Character�� BeginPlay �� ������ ��Բ� �ϴ¹��?
}
