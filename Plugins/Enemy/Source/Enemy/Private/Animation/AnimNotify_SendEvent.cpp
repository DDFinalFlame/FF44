// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify_SendEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

void UAnimNotify_SendEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp) return;

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor) return;

	// Owner�� Ability System Component�� ���� ���
	UAbilitySystemComponent* ASC = OwnerActor->FindComponentByClass<UAbilitySystemComponent>();
	if (!ASC) return;

	// FGameplayEventData ����
	FGameplayEventData EventData;
	EventData.Instigator = OwnerActor;
	EventData.Target = nullptr;
	EventData.EventTag = EventTag;

	// Event ����
	ASC->HandleGameplayEvent(EventTag, &EventData);
}
