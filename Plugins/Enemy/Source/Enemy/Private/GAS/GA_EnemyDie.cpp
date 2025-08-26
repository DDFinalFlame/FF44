// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_EnemyDie.h"

#include "AIController.h"
#include "BaseEnemy.h"

UGA_EnemyDie::UGA_EnemyDie()
{
}

void UGA_EnemyDie::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	/* 관련 요소 valid 체크 **/
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
	//Enemy->OnDeath();

	/* 몽타주 종료 처리는 AnimNotify를 통해 **/
	if (UAnimMontage* Montage = Enemy->GetDieMontage())
	{
		AnimInstance->Montage_Play(Montage);
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UGA_EnemyDie::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
}

void UGA_EnemyDie::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	///* 어빌리티 종료 **/
	//EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);

	//if (ABaseEnemy* Enemy = Cast<ABaseEnemy>(CurrentActorInfo->AvatarActor.Get()))
	//{
	//	Enemy->EndDeath();
	//}
}
