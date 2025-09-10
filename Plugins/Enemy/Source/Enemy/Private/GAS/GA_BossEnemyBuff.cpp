// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_BossEnemyBuff.h"

#include "AbilitySystemGlobals.h" 
#include "AbilitySystemBlueprintLibrary.h"
#include "BaseEnemy.h"

void UGA_BossEnemyBuff::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	/* 관련 요소 valid 체크 **/
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC) { return; }

	if (TriggerEventData->OptionalObject && TriggerEventData->Instigator.Get())
	{
		// 데미지 Effect 적용
		const UClass* EffectUClass = Cast<UClass>(TriggerEventData->OptionalObject);

		if (EffectUClass && EffectUClass->IsChildOf(UGameplayEffect::StaticClass()))
		{
			TSubclassOf<UGameplayEffect> EffectClass = const_cast<UClass*>(EffectUClass);

			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			ContextHandle.AddInstigator(const_cast<AActor*>(TriggerEventData->Instigator.Get()), const_cast<AActor*>(TriggerEventData->Instigator.Get()));
			ContextHandle.AddSourceObject(TriggerEventData->Instigator.Get());

			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
				EffectClass,
				1.0f,
				ContextHandle
			);

			FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Data.AttackPower"));
			SpecHandle.Data->SetSetByCallerMagnitude(DamageTag, TriggerEventData->EventMagnitude);

			ASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), ASC);
		}

		// FX Effect 적용
		if (EvadeEffectClass)
		{
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EvadeEffectClass, 1.0f, ContextHandle);

			if (SpecHandle.IsValid())
			{
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}
