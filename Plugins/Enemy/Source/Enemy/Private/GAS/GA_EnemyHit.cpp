// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_EnemyHit.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "BaseEnemy.h"
#include "MonsterAttributeSet.h"

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


	// Target Source ASC ��������
	UAbilitySystemComponent* TargetASC = ActorInfo->AbilitySystemComponent.Get();

	if (!TriggerEventData) { return; }
	const AActor* InstigatorActor = TriggerEventData->Instigator.Get();

	if (!InstigatorActor) { return; }
	UAbilitySystemComponent* AttackerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(const_cast<AActor*>(InstigatorActor));

	if (!AttackerASC) { return; }

	FGameplayEffectContextHandle ContextHandle = AttackerASC->MakeEffectContext();
	ContextHandle.AddInstigator(const_cast<AActor*>(InstigatorActor), nullptr);

	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		ContextHandle.AddSourceObject(ActorInfo->AvatarActor.Get());
	}

	if (AttackerASC && TargetASC && TargetASC->GetOwnerRole() == ROLE_Authority)
	{
		// ������ '������ ASC'�� ����� (Source=�÷��̾�)
		FGameplayEffectSpecHandle Spec = AttackerASC->MakeOutgoingSpec(HitEffect, 1.f, ContextHandle);
		if (Spec.IsValid())
		{
			// ByCaller �� �� �Ÿ� �ƹ� �͵� ���� ����,
			// EC�� ĸó(Attack=Source, Defense=Target)���� ����
			AttackerASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);

			const UMonsterAttributeSet* MyAttrSet = TargetASC->GetSet<UMonsterAttributeSet>();
			if (MyAttrSet)
			{
				float CurrentHealth = MyAttrSet->GetHealth();
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Value: %f"), CurrentHealth));
				int a = 0;
			}
		}
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
