// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_SevarogIntroStart.h"
#include "BaseEnemy.h"
#include <Abilities/Tasks/AbilityTask_PlayMontageAndWait.h>

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UGA_SevarogIntroStart::UGA_SevarogIntroStart()
{

}

void UGA_SevarogIntroStart::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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

	UAbilityTask_PlayMontageAndWait* Task =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,            // �½�ũ �̸�
			TargetMontage,        // ������ ��Ÿ��
			1.0f,                 // ��� �ӵ�
			NAME_None,            // StartSection
			false,                // StopWhenAbilityEnds
			1.0f                  // RootMotionScale
		);

	if (Task)
	{
		// ��������Ʈ ���ε�
		Task->OnCompleted.AddDynamic(this, &UGA_SevarogIntroStart::OnMontageCompleted);
		Task->OnInterrupted.AddDynamic(this, &UGA_SevarogIntroStart::OnMontageCancelled);
		Task->OnCancelled.AddDynamic(this, &UGA_SevarogIntroStart::OnMontageCancelled);
		Task->OnBlendOut.AddDynamic(this, &UGA_SevarogIntroStart::OnMontageBlendOut);

		Task->ReadyForActivation(); // �½�ũ ����
	}
	else
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
	}

}

void UGA_SevarogIntroStart::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);

	ABaseEnemy* Enemy = Cast<ABaseEnemy>(CurrentActorInfo->AvatarActor.Get());
	if (Enemy)
	{
		Enemy->EndCurrentBehavior();
	}

	// 1������ ����
	if(AAIController* AIController = Cast<AAIController>(Enemy->GetController()))
	{
		if (UBlackboardComponent* BB = AIController->GetBlackboardComponent())
		{
			BB->SetValueAsInt(FName("Phase"), 1);
		}
	}
}

void UGA_SevarogIntroStart::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);

	ABaseEnemy* Enemy = Cast<ABaseEnemy>(CurrentActorInfo->AvatarActor.Get());
	if (Enemy)
	{
		Enemy->EndCurrentBehavior();
	}

	// 1������ ����
	if (AAIController* AIController = Cast<AAIController>(Enemy->GetController()))
	{
		if (UBlackboardComponent* BB = AIController->GetBlackboardComponent())
		{
			BB->SetValueAsInt(FName("Phase"), 1);
		}
	}
}

void UGA_SevarogIntroStart::OnMontageBlendOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);

	ABaseEnemy* Enemy = Cast<ABaseEnemy>(CurrentActorInfo->AvatarActor.Get());
	if (Enemy)
	{
		Enemy->EndCurrentBehavior();
	}

	// 1������ ����
	if (AAIController* AIController = Cast<AAIController>(Enemy->GetController()))
	{
		if (UBlackboardComponent* BB = AIController->GetBlackboardComponent())
		{
			BB->SetValueAsInt(FName("Phase"), 1);
		}
	}
}