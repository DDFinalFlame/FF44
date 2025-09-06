// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss/GA_BossAttack_Swing1.h"
#include "Monster/MonsterCharacter.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "MonsterTags.h"

UGA_BossAttack_Swing1::UGA_BossAttack_Swing1()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    // 태그 등록 (필요 시 수정 가능)
    FGameplayTagContainer Tags;
    Tags.AddTag(MonsterTags::Ability_Boss_Swing1);
    SetAssetTags(Tags);
}

void UGA_BossAttack_Swing1::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    AMonsterCharacter* Boss = Cast<AMonsterCharacter>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr);
    if (!Boss) { EndAbility(Handle, ActorInfo, ActivationInfo, true, true); return; }

    UAnimMontage* Montage = AttackMontage.IsValid() ? AttackMontage.Get() : nullptr;
    if (!Montage)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, Montage, 1.f, NAME_None, false);

    if (!Task) { EndAbility(Handle, ActorInfo, ActivationInfo, true, false); return; }

    Task->OnCompleted.AddDynamic(this, &UGA_BossAttack_Swing1::OnMontageCompleted);
    Task->OnInterrupted.AddDynamic(this, &UGA_BossAttack_Swing1::OnMontageCancelled);
    Task->OnCancelled.AddDynamic(this, &UGA_BossAttack_Swing1::OnMontageCancelled);
    Task->ReadyForActivation();
}

void UGA_BossAttack_Swing1::OnMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_BossAttack_Swing1::OnMontageCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
