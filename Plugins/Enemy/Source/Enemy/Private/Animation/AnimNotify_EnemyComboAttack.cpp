// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify_EnemyComboAttack.h"

#include "BaseEnemy.h"

void UAnimNotify_EnemyComboAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp) return;

	// �޺� ���� �����Ǹ� montage �̾��ֱ�
	if (AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (IEnemyWeaponControl* WeaponControl = Cast<IEnemyWeaponControl>(OwnerActor))
		{
			if (WeaponControl->IsAttackSuccessful())
			{
				if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
				{
					AnimInstance->Montage_SetNextSection(FName("Combo1"), FName("Combo2"));
				}
			}
		}
	}
}
