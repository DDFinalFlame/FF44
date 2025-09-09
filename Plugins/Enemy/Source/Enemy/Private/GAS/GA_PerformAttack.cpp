// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_PerformAttack.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "BaseEnemy.h"
#include <Abilities/Tasks/AbilityTask_PlayMontageAndWait.h>

UGA_PerformAttack::UGA_PerformAttack()
{

}

void UGA_PerformAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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

	/* Attack Tag에 따라 Montage 가져오기 **/
	FGameplayTagContainer TargetTags;
	TargetTags.AddTag(AbilityTag);
	UAnimMontage* TargetMontage = Enemy->GetAttackMontage(TargetTags);

	UAbilityTask_PlayMontageAndWait* Task =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,            // 태스크 이름
			TargetMontage,        // 실행할 몽타주
			1.0f,                 // 재생 속도
			NAME_None,            // StartSection
			false,                // StopWhenAbilityEnds
			1.0f                  // RootMotionScale
		);

	if (Task)
	{
		// 델리게이트 바인딩
		Task->OnCompleted.AddDynamic(this, &UGA_PerformAttack::OnMontageCompleted);
		Task->OnInterrupted.AddDynamic(this, &UGA_PerformAttack::OnMontageCancelled);
		Task->OnCancelled.AddDynamic(this, &UGA_PerformAttack::OnMontageCancelled);
		Task->OnBlendOut.AddDynamic(this, &UGA_PerformAttack::OnMontageBlendOut);

		Task->ReadyForActivation(); // 태스크 시작
	}
	else
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
	}

	//   /* 몽타주 끝날 때 델리게이트 연결 **/
	//   FOnMontageEnded MontageDelegate;
	//   MontageDelegate.BindUObject(this, &UGA_PerformAttack::OnMontageEnded);

	   //FOnMontageBlendedInEnded MontageBlendedInDeletage;
	//   MontageBlendedInDeletage.BindUObject(this, &UGA_PerformAttack::OnMontageBlendedIn);

	//   FOnMontageBlendingOutStarted MontageBlendedOutDeletage;
	//   MontageBlendedOutDeletage.BindUObject(this, &UGA_PerformAttack::OnMontageBlendedOut);


	//   FAlphaBlendArgs AlphaBlendArgs;
	//   AlphaBlendArgs.BlendTime = BlendingTime;

	//   AnimInstance->Montage_PlayWithBlendIn(TargetMontage, AlphaBlendArgs);
	//   AnimInstance->Montage_SetEndDelegate(MontageDelegate, TargetMontage);
	//   AnimInstance->Montage_SetBlendedInDelegate(MontageBlendedInDeletage, TargetMontage);
	//   AnimInstance->Montage_SetBlendingOutDelegate(MontageBlendedOutDeletage, TargetMontage);

}

void UGA_PerformAttack::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);

	ABaseEnemy* Enemy = Cast<ABaseEnemy>(CurrentActorInfo->AvatarActor.Get());
	if (Enemy)
	{
		Enemy->EndCurrentBehavior();
	}
}

void UGA_PerformAttack::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);

	ABaseEnemy* Enemy = Cast<ABaseEnemy>(CurrentActorInfo->AvatarActor.Get());
	if (Enemy)
	{
		Enemy->EndCurrentBehavior();
	}
}

void UGA_PerformAttack::OnMontageBlendOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);

	ABaseEnemy* Enemy = Cast<ABaseEnemy>(CurrentActorInfo->AvatarActor.Get());
	if (Enemy)
	{
		Enemy->EndCurrentBehavior();
	}
}

//void UGA_PerformAttack::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
//{
//    //UE_LOG(LogTemp, Log, TEXT("OnMontageEnded"));
//
//    UE_LOG(LogTemp, Warning, TEXT("Montage Ended: %s, Interrupted: %s"),
//        *Montage->GetName(),
//        bInterrupted ? TEXT("true") : TEXT("false"));
//
//    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
//
//    ABaseEnemy* Enemy = Cast<ABaseEnemy>(CurrentActorInfo->AvatarActor.Get());
//    if (Enemy)
//    {
//	    Enemy->EndCurrentBehavior();
//    }
//}
//
//void UGA_PerformAttack::OnMontageBlendedIn(UAnimMontage* Montage)
//{
//    //UE_LOG(LogTemp, Log, TEXT("OnMontageBlendedIn"));
//}
//
//void UGA_PerformAttack::OnMontageBlendedOut(UAnimMontage* Montage, bool bSth)
//{
//    //UE_LOG(LogTemp, Log, TEXT("OnMontageBlendedOut"));
//
//}
