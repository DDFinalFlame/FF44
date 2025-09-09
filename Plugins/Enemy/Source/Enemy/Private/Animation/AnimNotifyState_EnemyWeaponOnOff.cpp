// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifyState_EnemyWeaponOnOff.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Interfaces/EnemyWeaponControl.h"


class UAbilitySystemComponent;

UAnimNotifyState_EnemyWeaponOnOff::UAnimNotifyState_EnemyWeaponOnOff(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UAnimNotifyState_EnemyWeaponOnOff::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (IEnemyWeaponControl* WeaponControl = Cast<IEnemyWeaponControl>(OwnerActor))
		{
			WeaponControl->ActivateWeaponCollision(WeaponType);
		}
	}
}

void UAnimNotifyState_EnemyWeaponOnOff::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	//Super::NotifyEnd(MeshComp, Animation, EventReference);

	/* 필요 요소 확인 **/
	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor) { return; }

	IEnemyWeaponControl* WeaponControl = Cast<IEnemyWeaponControl>(OwnerActor);
	if (!WeaponControl) { return; }

	/* 콤보 플레이 확인 **/
	if (HasCombo && WeaponControl->IsAttackSuccessful())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage();

			/* 섹션 간 블랜딩 필요 여부에 확인해서 처리 **/
			if (!bShouldBlend)
			{
				FName CurSection = AnimInstance->Montage_GetCurrentSection(AnimInstance->GetCurrentActiveMontage());
				UE_LOG(LogTemp, Log, TEXT("Current Section: %s"), *CurSection.ToString());

				AnimInstance->Montage_JumpToSection(NextSectionName, CurrentMontage);
			}
			else
			{
				//AnimInstance->Montage_SetNextSection(CurrentSectionName, NextSectionName);


				/* 섹션 간 블랜딩 작업
				 * 중지 -> 블랜딩 설정을 가지고 다시 실행 -> 섹션 설정 **/

				AnimInstance->Montage_Stop(0.5f);

				FAlphaBlendArgs BlendInArgs;
				BlendInArgs.BlendTime = 0.5f;
				BlendInArgs.BlendOption = EAlphaBlendOption::Linear;

				float Duration = AnimInstance->Montage_PlayWithBlendIn(
					CurrentMontage,
					BlendInArgs,
					1.0f, // Play Rate
					EMontagePlayReturnType::MontageLength,
					0.0f, // Start time
					false // bStopAllMontages
				);

				if (Duration > 0.f)
				{
					AnimInstance->Montage_JumpToSection(NextSectionName, CurrentMontage);
				}
			}
		}
	}
	else
	{
		// Owner가 Ability System Component를 가진 경우
		UAbilitySystemComponent* ASC = OwnerActor->FindComponentByClass<UAbilitySystemComponent>();
		if (!ASC) return;

		// FGameplayEventData 생성
		FGameplayEventData EventData;
		EventData.Instigator = OwnerActor;
		EventData.Target = nullptr;
		EventData.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Enemy.Event.EndAbility"));

		// Event 전송
		ASC->HandleGameplayEvent(EventData.EventTag, &EventData);
	}

	/* 콤보 확인 후 Deactive **/
	WeaponControl->DeactivateWeaponCollision(WeaponType);
}
