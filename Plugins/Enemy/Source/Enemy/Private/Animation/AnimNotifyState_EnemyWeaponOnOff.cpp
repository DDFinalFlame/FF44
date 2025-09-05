// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifyState_EnemyWeaponOnOff.h"

#include "Interfaces/EnemyWeaponControl.h"


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
			WeaponControl->ActivateWeaponCollision();
		}
	}
}

void UAnimNotifyState_EnemyWeaponOnOff::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	/* �ʿ� ��� Ȯ�� **/
	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor) { return; }

	IEnemyWeaponControl* WeaponControl = Cast<IEnemyWeaponControl>(OwnerActor);
	if (!WeaponControl) { return; }

	/* �޺� �÷��� Ȯ�� **/
	if (HasCombo && WeaponControl->IsAttackSuccessful())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			/* ���� �� ���� �ʿ� ���ο� Ȯ���ؼ� ó�� **/
			if (!bShouldBlend)
			{
				AnimInstance->Montage_SetNextSection(CurrentSectionName, NextSectionName);
			}
			else
			{
				/* ���� �� ���� �۾�
				 * ���� -> ���� ������ ������ �ٽ� ���� -> ���� ���� **/
				UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage();

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

	/* �޺� Ȯ�� �� Deactive **/
	WeaponControl->DeactivateWeaponCollision();
}
