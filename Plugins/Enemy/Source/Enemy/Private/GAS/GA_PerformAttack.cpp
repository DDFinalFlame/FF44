// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_PerformAttack.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "BaseEnemy.h"
#include <Abilities/Tasks/AbilityTask_PlayMontageAndWait.h>

UGA_PerformAttack::UGA_PerformAttack()
{
    /* ��� ����Ʈ���� ������ tag �ο� **/
    if (AbilityTag.IsValid())
    {
        AbilityTags.AddTag(AbilityTag);
    }
}

void UGA_PerformAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
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

	FOnMontageBlendedInEnded MontageBlendedInDeletage;
    MontageBlendedInDeletage.BindUObject(this, &UGA_PerformAttack::OnMontageBlendedIn);

    FOnMontageBlendingOutStarted MontageBlendedOutDeletage;
    MontageBlendedOutDeletage.BindUObject(this, &UGA_PerformAttack::OnMontageBlendedOut);


    FAlphaBlendArgs AlphaBlendArgs;
    AlphaBlendArgs.BlendTime = BlendingTime;

    AnimInstance->Montage_PlayWithBlendIn(AttackAnimMontage, AlphaBlendArgs);
    AnimInstance->Montage_SetEndDelegate(MontageDelegate, AttackAnimMontage);
    AnimInstance->Montage_SetBlendedInDelegate(MontageBlendedInDeletage, AttackAnimMontage);
    AnimInstance->Montage_SetBlendingOutDelegate(MontageBlendedOutDeletage, AttackAnimMontage);
    /* **/

}

void UGA_PerformAttack::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    ///* ��� ����Ʈ���� ������ tag �ο� **/
    //if (AbilityTag.IsValid())
    //{
    //    AbilityTags.AddTag(AbilityTag);
    //}

	Super::OnGiveAbility(ActorInfo, Spec);
}

void UGA_PerformAttack::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    UE_LOG(LogTemp, Log, TEXT("OnMontageEnded"));

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);

    ABaseEnemy* Enemy = Cast<ABaseEnemy>(CurrentActorInfo->AvatarActor.Get());
    if (Enemy)
    {
	    Enemy->EndCurrentBehavior();
    }
}

void UGA_PerformAttack::OnMontageBlendedIn(UAnimMontage* Montage)
{
    UE_LOG(LogTemp, Log, TEXT("OnMontageBlendedIn"));
}

void UGA_PerformAttack::OnMontageBlendedOut(UAnimMontage* Montage, bool bSth)
{
    UE_LOG(LogTemp, Log, TEXT("OnMontageBlendedOut"));

}
