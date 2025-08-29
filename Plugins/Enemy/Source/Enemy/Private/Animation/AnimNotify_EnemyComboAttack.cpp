// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify_EnemyComboAttack.h"

#include "BaseEnemy.h"

void UAnimNotify_EnemyComboAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp) return;

	// 콤보 조건 성립되면 montage 이어주기
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
