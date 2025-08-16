#include "GA_MonsterAttack.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemComponent.h"
#include "MonsterCharacter.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

static FGameplayTag TAG_Ability_Attack() { return FGameplayTag::RequestGameplayTag(TEXT("Ability.Monster.Attack")); }
UGA_MonsterAttack::UGA_MonsterAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;


    FGameplayTagContainer Tags; Tags.AddTag(TAG_Ability_Attack()); SetAssetTags(Tags);

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

    if (UMonsterDefinition* Def = MC->GetMonsterDef())
    {
        if (!Def->AttackMontage.IsValid()) Def->AttackMontage.LoadSynchronous();
        AttackMontage = Def->AttackMontage.Get();
    }

    if (!AttackMontage)
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