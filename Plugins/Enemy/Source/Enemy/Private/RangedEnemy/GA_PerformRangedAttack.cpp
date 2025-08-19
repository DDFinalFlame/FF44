// Fill out your copyright notice in the Description page of Project Settings.


#include "RangedEnemy/GA_PerformRangedAttack.h"

#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "GameFramework/Character.h"
#include "Interfaces/RangeAttack.h"
#include "Projectile/ProjectileBase.h"

void UGA_PerformRangedAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// �⺻ Attack ���� ( Anim Montage ���� )
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Ranged Attack ���� ó��
	/* Enemy�� RangeAttack �������̽��� ���� Spawn�� �ʿ��� ���� ��� **/
	IRangeAttack* rangeAttackCharacter = Cast<IRangeAttack>(ActorInfo->AvatarActor.Get());
	if (!rangeAttackCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FVector Location = rangeAttackCharacter->GetMuzzleLocation();
	FVector Direction = rangeAttackCharacter->GetMuzzleDirection();
	FRotator Rotation = Direction.Rotation();

	/* Projectile ���� **/
	if (!ProjectileClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* OwnerActor = ActorInfo->OwnerActor.Get();
	APawn* InstigatorPawn = Cast<APawn>(ActorInfo->AvatarActor.Get());

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerActor;
	SpawnParams.Instigator = InstigatorPawn;

	AProjectileBase* Projectile = GetWorld()->SpawnActor<AProjectileBase>(ProjectileClass, Location, Rotation, SpawnParams);
	if (Projectile)
	{
		//Projectile->SetReplicates(true);
		//Projectile->SetReplicateMovement(true);
		Projectile->FireInDirection(Rotation.Vector());
	}
}
