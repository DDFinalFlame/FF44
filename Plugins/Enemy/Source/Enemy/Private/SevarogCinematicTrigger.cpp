// Fill out your copyright notice in the Description page of Project Settings.


#include "SevarogCinematicTrigger.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/BoxComponent.h"

void ASevarogCinematicTrigger::TriggerOnce(AActor* _OtherActor)
{
	bTriggered = true;

	if (!TargetBoss && bAutoResolveBoss)
	{
		TryResolveBoss(); // 마지막으로 한 번 더 시도
	}

	if (TargetBoss)
	{
		SendBossIntroEvent();

	}

	if (OnCinematicTriggered.IsBound())
	{
		OnCinematicTriggered.Broadcast(_OtherActor);
	}
	BP_OnCinematicTriggered(_OtherActor);

	if (bDestroyAfterTrigger)
	{
		Destroy();
	}
	else if (bOneShot)
	{
		Box->SetGenerateOverlapEvents(false);
		SetActorEnableCollision(false);
		SetActorHiddenInGame(true);
	}

}
