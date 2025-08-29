// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifyState_EnemyCheckWeapon.h"

UAnimNotifyState_EnemyCheckWeapon::UAnimNotifyState_EnemyCheckWeapon(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UAnimNotifyState_EnemyCheckWeapon::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime);

	if (AActor* OwnerActor = MeshComp->GetOwner())
	{
		//if (IEnemyWeaponControl* WeaponControl = Cast<IEnemyWeaponControl>(OwnerActor))
		//{
		//	WeaponControl->DeactivateWeaponCollision();
		//}
	}
}
