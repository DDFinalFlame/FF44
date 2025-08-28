#include "Player/BasePlayerState.h"
#include "AbilitySystemComponent.h"

ABasePlayerState::ABasePlayerState()
{
    ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
    ASC->SetIsReplicated(true);                         // 컴포넌트 복제 활성화
    ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed); // Full/Mixed/Minimal 중 선택
}
