// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_EnemyEvadeEnd.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Interfaces/EnemyEvade.h"

void UGA_EnemyEvadeEnd::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                        const FGameplayEventData* TriggerEventData)
{

    // Valid 체크
    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    IEnemyEvade* Enemy = Cast<IEnemyEvade>(ActorInfo->AvatarActor.Get());
    if (!Enemy)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // EvadeEnd 이벤트 발행
    FGameplayEventData EventData;

    EventData.EventTag = FGameplayTag::RequestGameplayTag("Enemy.Event.EvadeEnd");
    EventData.Instigator = ActorInfo->AvatarActor.Get();
    EventData.Target = ActorInfo->AvatarActor.Get();

    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        ActorInfo->AvatarActor.Get(),
        EventData.EventTag,
        EventData
    );

    // 보이게
    Enemy->ToggleDissolve(false);

    
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);

}
