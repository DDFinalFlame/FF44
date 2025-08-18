// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_PerformAttack.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "BaseEnemy.h"
#include <Abilities/Tasks/AbilityTask_PlayMontageAndWait.h>

void UGA_PerformAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

	if (AbilityTag.IsValid() && !AbilityTags.HasTagExact(AbilityTag))
    {
        AbilityTags.AddTag(AbilityTag);
    }

    ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
    if (!Character || !AttackAnimMontage)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
    if (!AnimInstance)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    /* ��Ÿ�� ���� �� ��������Ʈ ���� **/
    FOnMontageEnded MontageDelegate;
    MontageDelegate.BindUObject(this, &UGA_PerformAttack::OnMontageEnded);
    AnimInstance->Montage_Play(AttackAnimMontage);
    AnimInstance->Montage_SetEndDelegate(MontageDelegate, AttackAnimMontage);
}

void UGA_PerformAttack::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

    /* ��� ����Ʈ���� ������ tag �ο� **/
    if (AbilityTag.IsValid())
    {
        AbilityTags.AddTag(AbilityTag);
    }
}

void UGA_PerformAttack::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}
