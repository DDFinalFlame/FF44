// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_BossEnemyHit.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "MonsterAttributeSet.h"
#include "BaseBoss.h"
#include "ESGameplayTags.h"

UGA_BossEnemyHit::UGA_BossEnemyHit()
{
}

void UGA_BossEnemyHit::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	/* 관련 요소 valid 체크 **/
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ABaseBoss* Enemy = Cast<ABaseBoss>(ActorInfo->AvatarActor.Get());
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
	IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(ActorInfo->OwnerActor.Get());
	UAbilitySystemComponent* TargetASC = nullptr;
	if (AbilitySystemInterface)
	{
		TargetASC = AbilitySystemInterface->GetAbilitySystemComponent();
	}
	if (!TargetASC) { return; }


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
				float MaxHealth = MyAttrSet->GetMaxHealth();
				Enemy->SetPhase(CurrentHealth, MaxHealth);
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Value: %f"), CurrentHealth));
			}
		}
	}

	// Evade 상태일 떄만 몽타주 실행
	if (TargetASC->HasMatchingGameplayTag(SLGameplayTags::Enemy_Boss_Attack_EvadeStart))
	{

		if (UAnimMontage* Montage = Enemy->GetHitMontage(EHitDirection::Front))
		{
			// 몽타주 실행
			FOnMontageEnded MontageDelegate;
			MontageDelegate.BindUObject(this, &UGA_BossEnemyHit::OnMontageEnded);
			AnimInstance->Montage_Play(Montage);
			AnimInstance->Montage_SetEndDelegate(MontageDelegate, Montage);
		}
	}
	else
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
		return;
	}

}

void UGA_BossEnemyHit::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	/* 어빌리티 종료 **/
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}
