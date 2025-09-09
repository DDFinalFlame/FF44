// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_PerformAttack_TaskDriven.h"

#include "BaseEnemy.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

UGA_PerformAttack_TaskDriven::UGA_PerformAttack_TaskDriven()
{
}

void UGA_PerformAttack_TaskDriven::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                   const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                   const FGameplayEventData* TriggerEventData)
{
    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    ABaseEnemy* Enemy = Cast<ABaseEnemy>(ActorInfo->AvatarActor.Get());
    if (!Enemy)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UAnimInstance* AnimInstance = Enemy->GetMesh()->GetAnimInstance();
    if (!AnimInstance)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // 몽타주 실행
    /* Attack Tag에 따라 Montage 가져오기 **/
    FGameplayTagContainer TargetTags;
    TargetTags.AddTag(AbilityTag);
    UAnimMontage* TargetMontage = Enemy->GetAttackMontage(TargetTags);

    FAlphaBlendArgs AlphaBlendArgs;
    AlphaBlendArgs.BlendTime = BlendingTime;
    AnimInstance->Montage_PlayWithBlendIn(TargetMontage, AlphaBlendArgs);

    // 이벤트 대기 ( 애님 노티파이에서 이벤트 태그 적용 )
    FGameplayTag TagA = FGameplayTag::RequestGameplayTag(TEXT("Enemy.Event.EndAbility"));
    UAbilityTask_WaitGameplayEvent* TaskA = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this, TagA, nullptr, true, true);
    TaskA->EventReceived.AddDynamic(this, &UGA_PerformAttack_TaskDriven::OnEndTask);
    TaskA->ReadyForActivation();
}

void UGA_PerformAttack_TaskDriven::OnEndTask(FGameplayEventData Payload)
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);

    ABaseEnemy* Enemy = Cast<ABaseEnemy>(CurrentActorInfo->AvatarActor.Get());
    if (Enemy)
    {
        Enemy->EndCurrentBehavior();
    }
}
