// Fill out your copyright notice in the Description page of Project Settings.


#include "SummonedEnemy.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "MonsterAttributeSet.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Interfaces/EnemyAIState.h"
#include "Kismet/GameplayStatics.h"

ASummonedEnemy::ASummonedEnemy()
{
	// ĸ���� ������ �������� ���� ����
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Overlap);
}

void ASummonedEnemy::BeginPlay()
{
	Super::BeginPlay();
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ASummonedEnemy::OnCapsuleBeginOverlap);

	AudioComponent = UGameplayStatics::SpawnSoundAttached(
		SoundAsset,               // ����� SoundBase
		GetMesh(),       // �θ� ������Ʈ
		SoundSocketName,          // ���� �̸�
		FVector::ZeroVector, // ���� ��ġ ������
		FRotator::ZeroRotator, // ���� ȸ�� ������
		EAttachLocation::SnapToTarget, // �ٴ� ���
		true,
		0.7f,
		0.7f
	);

}

void ASummonedEnemy::OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AudioComponent)
	{
		AudioComponent->Stop();
	}
	//UE_LOG(LogTemp, Log, TEXT("Begin Overlap with: %s"), *OtherActor->GetName());
	if (IsValid(OtherActor) && OtherActor->ActorHasTag(FName("Player")))
	{
		// �÷��̾� �浹 - ������ ����
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
		// ���� �浹 - ( ���� -> �Ҹ� )
		/* Ư�� behavior( Patrol )���� �����ϵ��� **/
		{
			IEnemyAIState* AIController = Cast<IEnemyAIState>(GetController());
			if (!AIController) { return; }
			if (AIController->GetCurrentBehavior() != EAIBehavior::Patrol) { return; }
		}
	
		/* ���� Event ȣ��**/
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

		/* 0.5�� �� �Ҹ� **/
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
