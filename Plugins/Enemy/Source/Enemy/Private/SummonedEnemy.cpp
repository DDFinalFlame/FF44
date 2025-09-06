// Fill out your copyright notice in the Description page of Project Settings.


#include "SummonedEnemy.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "MonsterAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Interfaces/EnemyAIState.h"

ASummonedEnemy::ASummonedEnemy()
{
	// 캡슐을 오버랩 전용으로 설정 가능
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Overlap);
}

void ASummonedEnemy::BeginPlay()
{
	Super::BeginPlay();
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ASummonedEnemy::OnCapsuleBeginOverlap);

}

void ASummonedEnemy::OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//UE_LOG(LogTemp, Log, TEXT("Begin Overlap with: %s"), *OtherActor->GetName());
	if (IsValid(OtherActor) && OtherActor->ActorHasTag(FName("Player")))
	{
		// 플레이어 충돌 - 데미지 로직
		FGameplayEventData EventData;

		EventData.EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Player.Hit"));
		EventData.Instigator = this;
		EventData.Target = OtherActor;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			OtherActor,
			EventData.EventTag,
			EventData
		);
	}
	else if (OtherActor->ActorHasTag(FName("Sevarog")))
	{
		// 보스 충돌 - ( 버프 -> 소멸 )
		/* 특정 behavior( Patrol )에만 가능하도록 **/
		{
			IEnemyAIState* AIController = Cast<IEnemyAIState>(GetController());
			if (!AIController) { return; }
			if (AIController->GetCurrentBehavior() != EAIBehavior::Patrol) { return; }
		}
	
		/* 버프 Event 호출**/
		{
			FGameplayEventData EventData;

			EventData.EventTag = FGameplayTag::RequestGameplayTag(FName("Enemy.Event.Buff"));
			EventData.Instigator = this;
			EventData.Target = OtherActor;
			EventData.OptionalObject = BuffEffectCDO.Get();

			UAbilitySystemComponent* ASC = GetComponentByClass<UAbilitySystemComponent>();
			const UMonsterAttributeSet* A = Cast<UMonsterAttributeSet>(ASC->GetAttributeSet(UMonsterAttributeSet::StaticClass()));
			EventData.EventMagnitude = A->GetAttackPower();

			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
				OtherActor,
				EventData.EventTag,
				EventData
			);
		}

		/* 0.5초 후 소멸 **/
		{
			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			FTimerHandle DestroyTimerHandle;
			GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &ASummonedEnemy::OnDestroySummonedEnemy, 0.5f, false);

		}

	}
}

void ASummonedEnemy::OnDestroySummonedEnemy()
{
	FGameplayEventData EventData;
	EventData.EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Monster.Death"));
	EventData.Target = this;

	AbilitySystemComponent->HandleGameplayEvent(EventData.EventTag, &EventData);
}

void ASummonedEnemy::OnDeath()
{
	Super::OnDeath();

	StartDissolve();

}
