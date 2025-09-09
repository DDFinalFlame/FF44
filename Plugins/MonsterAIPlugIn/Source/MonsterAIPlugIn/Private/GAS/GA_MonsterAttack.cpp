#include "GAS/GA_MonsterAttack.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemComponent.h"
#include "Monster/MonsterCharacter.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "MonsterTags.h"

UGA_MonsterAttack::UGA_MonsterAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;


    FGameplayTagContainer Tags; Tags.AddTag(MonsterTags::Ability_Attack); SetAssetTags(Tags);

}

void UGA_MonsterAttack::ActivateAbility(
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

    const AMonsterCharacter* MC = Cast<AMonsterCharacter>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr);
    if (!MC) { EndAbility(Handle, ActorInfo, ActivationInfo, true, true); return; }

    UAnimMontage* AttackMontage = nullptr;
    FName Section = NAME_None;

    if (UMonsterDefinition* Def = MC->GetMonsterDef())
    {
        bool bOk = false;
        if (!AttackKey.IsNone())
            bOk = Def->FindAttackByKey(AttackKey, AttackMontage, Section);
        else
            bOk = Def->PickRandomAttack(AttackMontage, Section);

        if (!bOk || !AttackMontage)
        {
            EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
            return;
        }
    }
    else
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // 섹션이 필요 없으면 NAME_None 전달(몽타주 전체 재생)
    auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, AttackMontage, 1.f, NAME_None, false);

    if (!Task) { EndAbility(Handle, ActorInfo, ActivationInfo, true, false); return; }

    Task->OnCompleted.AddDynamic(this, &UGA_MonsterAttack::OnMontageCompleted);
    Task->OnInterrupted.AddDynamic(this, &UGA_MonsterAttack::OnMontageCancelled);
    Task->OnCancelled.AddDynamic(this, &UGA_MonsterAttack::OnMontageCancelled);
    Task->ReadyForActivation();
}

void UGA_MonsterAttack::OnMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_MonsterAttack::OnMontageCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}