// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_EnemyDie.h"

#include "BaseEnemy.h"

UGA_EnemyDie::UGA_EnemyDie()
{
}

void UGA_EnemyDie::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	/* ���� ��� valid üũ **/
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

	/* State **/
	Enemy->ChangeState(EAIBehavior::Die);
	Enemy->OnDeath();

	/* ��Ÿ�� ���� ó���� AnimNotify�� ���� **/
	if (UAnimMontage* Montage = Enemy->GetDieMontage())
	{
		AnimInstance->Montage_Play(Montage);
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}