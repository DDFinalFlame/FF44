// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_EnemyHit.h"
#include "BaseEnemy.h"

UGA_EnemyHit::UGA_EnemyHit()
{
    /* ��� ����Ʈ���� ������ tag �ο� **/
    if (EventTag.IsValid())
    {
        AbilityTags.AddTag(EventTag);
    }
}

void UGA_EnemyHit::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    ABaseEnemy* Character = Cast<ABaseEnemy>(ActorInfo->AvatarActor.Get());
    if (!Character || !HitAnimMontage)
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
    MontageDelegate.BindUObject(this, &UGA_EnemyHit::OnMontageEnded);
    AnimInstance->Montage_Play(HitAnimMontage);
    AnimInstance->Montage_SetEndDelegate(MontageDelegate, HitAnimMontage);

    /* Effect ���� **/
    FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(HitEffect, GetAbilityLevel());
    if (SpecHandle.IsValid())
    {
        UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
        ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
    }

}

void UGA_EnemyHit::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    Super::OnGiveAbility(ActorInfo, Spec);
}

void UGA_EnemyHit::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    /* �����Ƽ ���� **/
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}
