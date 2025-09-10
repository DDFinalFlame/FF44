// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_EnemyEvade.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Interfaces/EnemyEvade.h"

void UGA_EnemyEvade::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                     const FGameplayEventData* TriggerEventData)
{
    // Valid üũ
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

    // ù ��° �̺�Ʈ ��� ( �߰��� �¾��� )
    FGameplayTag TagA = FGameplayTag::RequestGameplayTag(TEXT("Event.Monster.Hit"));
    UAbilityTask_WaitGameplayEvent* TaskA = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this, TagA, nullptr, true, true);
    TaskA->EventReceived.AddDynamic(this, &UGA_EnemyEvade::OnDamagedOnEvade);
    TaskA->ReadyForActivation();


    // �� ��° �̺�Ʈ ��� ( Evade ���� ���� )
    FGameplayTag TagB = FGameplayTag::RequestGameplayTag(TEXT("Enemy.Event.EvadeEnd"));
    UAbilityTask_WaitGameplayEvent* TaskB = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this, TagB, nullptr, true, true);
    TaskB->EventReceived.AddDynamic(this, &UGA_EnemyEvade::OnEvadeEnd);
    TaskB->ReadyForActivation();

    Enemy->ToggleDissolve(true);

    // GE ����
    if (EvadeEffectClass)
    {
        if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
        {
            FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
            FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EvadeEffectClass, 1.0f, ContextHandle);

            if (SpecHandle.IsValid())
            {
                ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            }
        }
    }
}

void UGA_EnemyEvade::OnDamagedOnEvade(FGameplayEventData Payload)
{
	// Ư�� Hit ���� ó��
    /* ����ȭ ���� ( ���߿� ��Ÿ ����Ʈ �� ,.. ? )**/
    IEnemyEvade* Enemy = Cast<IEnemyEvade>(CurrentActorInfo->AvatarActor.Get());
    if (Enemy)
    {
        Enemy->ToggleDissolve(false);
    }

    /*GE ����**/
    if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Payload.Target.Get()))
    {
	    ASC->RemoveActiveGameplayEffectBySourceEffect(EvadeEffectClass, ASC, 1);
    }

    // �����Ƽ ����
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_EnemyEvade::OnEvadeEnd(FGameplayEventData Payload)
{
    /*GE ����**/
    if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Payload.Target.Get()))
    {
        ASC->RemoveActiveGameplayEffectBySourceEffect(EvadeEffectClass, ASC, 1);
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
