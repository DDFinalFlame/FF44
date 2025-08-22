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

	if (AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (IEnemyWeaponControl* WeaponControl = Cast<IEnemyWeaponControl>(OwnerActor))
		{
			WeaponControl->DeactivateWeaponCollision();
		}
	}
}
