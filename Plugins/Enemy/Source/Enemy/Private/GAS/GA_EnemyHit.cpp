// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_EnemyHit.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "BaseEnemy.h"
#include "MonsterAttributeSet.h"

UGA_EnemyHit::UGA_EnemyHit()
{
}

void UGA_EnemyHit::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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

	/* Damage �Ա� (AttributeSet ���� �����ͼ� Effect ���� ( MonsterAIPlugin ���� )) **/
	// Target Source ASC ��������
	UAbilitySystemComponent* TargetASC = ActorInfo->AbilitySystemComponent.Get();

	if (!TriggerEventData) { return; }
	const AActor* InstigatorActor = TriggerEventData->Instigator.Get();
	AttackerLocation = InstigatorActor->GetActorLocation();
	Enemy->SetRotationTarget(AttackerLocation);


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
			}
		}
	}


	/* Animation �� BT�� ���� Enemy State ó�� **/
	if (!Enemy->ChangeState(EAIBehavior::Hit))
	{
		/* �������� ������, �ǰ� �ִϸ��̼��� ������� �ʴ� Enemy **/
		// TO-DO : �氨�� �����ϰ� �׿� ���� ������ �ݰ��ϱ�
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
		return;
	}

	/* ��Ÿ�� ���� �� ��������Ʈ ���� **/
	FOnMontageEnded MontageDelegate;

	if (UAnimMontage* Montage = Enemy->GetHitMontage(EHitDirection::Front))
	{
		MontageDelegate.BindUObject(this, &UGA_EnemyHit::OnMontageEnded);
		AnimInstance->Montage_Play(Montage);
		AnimInstance->Montage_SetEndDelegate(MontageDelegate, Montage);
	}
	else
	{
		/* ���� ���� **/
		Enemy->SetEnemyState(EAIBehavior::Patrol);
		/* �����Ƽ ���� **/
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);

	}

}

void UGA_EnemyHit::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	/* ���� ���� **/
	if (ABaseEnemy* Enemy = Cast<ABaseEnemy>(CurrentActorInfo->AvatarActor.Get()))
	{
		Enemy->SetEnemyState(EAIBehavior::Patrol);
		//Enemy->SetRotationTarget(AttackerLocation);
	}

	/* �����Ƽ ���� **/
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);

}
