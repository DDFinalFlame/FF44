// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_EnemyEvade.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Interfaces/EnemyEvade.h"

void UGA_EnemyEvade::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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

    // 첫 번째 이벤트 대기 ( 중간에 맞았음 )
    FGameplayTag TagA = FGameplayTag::RequestGameplayTag(TEXT("Event.Monster.Hit"));
    UAbilityTask_WaitGameplayEvent* TaskA = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this, TagA, nullptr, true, true);
    TaskA->EventReceived.AddDynamic(this, &UGA_EnemyEvade::OnDamagedOnEvade);
    TaskA->ReadyForActivation();


    // 두 번째 이벤트 대기 ( Evade 정상 종료 )
    FGameplayTag TagB = FGameplayTag::RequestGameplayTag(TEXT("Enemy.Event.EvadeEnd"));
    UAbilityTask_WaitGameplayEvent* TaskB = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this, TagB, nullptr, true, true);
    TaskB->EventReceived.AddDynamic(this, &UGA_EnemyEvade::OnEvadeEnd);
    TaskB->ReadyForActivation();

    Enemy->ToggleDissolve(true);
}

void UGA_EnemyEvade::OnDamagedOnEvade(FGameplayEventData Payload)
{
	// 특수 Hit 관련 처리
    /* 투명화 종료 ( 나중에 기타 이펙트 뭐 ,.. ? )**/
    IEnemyEvade* Enemy = Cast<IEnemyEvade>(CurrentActorInfo->AvatarActor.Get());
    if (Enemy)
    {
        Enemy->ToggleDissolve(false);
    }

    // 어빌리티 종료
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_EnemyEvade::OnEvadeEnd(FGameplayEventData Payload)
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
