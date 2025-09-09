// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify_SendEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

void UAnimNotify_SendEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp) return;

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor) return;

	// Owner가 Ability System Component를 가진 경우
	UAbilitySystemComponent* ASC = OwnerActor->FindComponentByClass<UAbilitySystemComponent>();
	if (!ASC) return;

	// FGameplayEventData 생성
	FGameplayEventData EventData;
	EventData.Instigator = OwnerActor;
	EventData.Target = nullptr;
	EventData.EventTag = EventTag;

	// Event 전송
	ASC->HandleGameplayEvent(EventTag, &EventData);
}
