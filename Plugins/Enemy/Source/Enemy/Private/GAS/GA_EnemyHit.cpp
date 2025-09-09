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

	/* Damage 입기 (AttributeSet 정보 가져와서 Effect 적용 ( MonsterAIPlugin 참고 )) **/
	// Target Source ASC 가져오기
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
		// 스펙을 '공격자 ASC'로 만든다 (Source=플레이어)
		FGameplayEffectSpecHandle Spec = AttackerASC->MakeOutgoingSpec(HitEffect, 1.f, ContextHandle);
		if (Spec.IsValid())
		{
			// ByCaller 안 쓸 거면 아무 것도 넣지 말고,
			// EC의 캡처(Attack=Source, Defense=Target)에만 의존
			AttackerASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);

			const UMonsterAttributeSet* MyAttrSet = TargetASC->GetSet<UMonsterAttributeSet>();
			if (MyAttrSet)
			{
				float CurrentHealth = MyAttrSet->GetHealth();
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Value: %f"), CurrentHealth));
			}
		}
	}


	/* Animation 과 BT를 위한 Enemy State 처리 **/
	if (!Enemy->ChangeState(EAIBehavior::Hit))
	{
		/* 데미지는 입지만, 피격 애니메이션이 실행되지 않는 Enemy **/
		// TO-DO : 경감률 설정하고 그에 따라 데미지 반감하기
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
		return;
	}

	/* 몽타주 끝날 때 델리게이트 연결 **/
	FOnMontageEnded MontageDelegate;

	if (UAnimMontage* Montage = Enemy->GetHitMontage(EHitDirection::Front))
	{
		MontageDelegate.BindUObject(this, &UGA_EnemyHit::OnMontageEnded);
		AnimInstance->Montage_Play(Montage);
		AnimInstance->Montage_SetEndDelegate(MontageDelegate, Montage);
	}
	else
	{
		/* 상태 변경 **/
		Enemy->SetEnemyState(EAIBehavior::Patrol);
		/* 어빌리티 종료 **/
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);

	}

}

void UGA_EnemyHit::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	/* 상태 변경 **/
	if (ABaseEnemy* Enemy = Cast<ABaseEnemy>(CurrentActorInfo->AvatarActor.Get()))
	{
		Enemy->SetEnemyState(EAIBehavior::Patrol);
		//Enemy->SetRotationTarget(AttackerLocation);
	}

	/* 어빌리티 종료 **/
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);

}
