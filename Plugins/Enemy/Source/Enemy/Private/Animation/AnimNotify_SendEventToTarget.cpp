// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify_SendEventToTarget.h"

#include "BaseBoss.h"

void UAnimNotify_SendEventToTarget::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp) return;

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor) return;

	if (ABaseBoss* Boss = Cast<ABaseBoss>(OwnerActor))
	{
		Boss->SendEventToTarget(EventTag);
	}
}
