#include "Player/BasePlayerState.h"
#include "AbilitySystemComponent.h"
#include "InventorySystem/InventoryComponent.h"

ABasePlayerState::ABasePlayerState()
{
    ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
    ASC->SetIsReplicated(true);                         // ������Ʈ ���� Ȱ��ȭ
    ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed); // Full/Mixed/Minimal �� ����

    IC = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
    IC->SetIsReplicated(true);
}
