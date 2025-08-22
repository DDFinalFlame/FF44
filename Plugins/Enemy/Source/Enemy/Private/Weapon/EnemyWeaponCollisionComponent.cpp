#include "Weapon/EnemyWeaponCollisionComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

UEnemyWeaponCollisionComponent::UEnemyWeaponCollisionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
}


void UEnemyWeaponCollisionComponent::BeginPlay()
{
	Super::BeginPlay();

}


void UEnemyWeaponCollisionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsCollisionEnabeld)
	{
		CollisionTrace();
	}

}

void UEnemyWeaponCollisionComponent::CollisionTrace()
{
	TArray<FHitResult> OutHits;

	const FVector Start = WeaponMesh->GetSocketLocation(StartSocketName);
	const FVector End = WeaponMesh->GetSocketLocation(EndSocketName);

	bool const bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
		GetOwner(),
		Start,
		End,
		TraceRadius,
		TraceObjectTypes,
		false,
		IgnoredActors,
		DrawDebugType,
		OutHits,
		true);

	if (bHit)
	{
		for (const FHitResult& Hit : OutHits)
		{
			AActor* HitActor = Hit.GetActor();

			if (HitActor == nullptr)
			{
				continue;
			}

			if (CanHitActor(HitActor))
			{
				AlreadyHitActors.Add(HitActor);

				FGameplayEventData EventData;

				EventData.EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Player.Hit"));
				EventData.Instigator = GetOwner()->GetOwner();
				EventData.Target = HitActor;

				/* HitResult를 TargetData로 포장 **/
				EventData.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(Hit);

				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
					HitActor,
					EventData.EventTag,
					EventData
				);
			}
		}
	}
}

void UEnemyWeaponCollisionComponent::ActivateCollision()
{
	AlreadyHitActors.Empty();
	bIsCollisionEnabeld = true;
}

void UEnemyWeaponCollisionComponent::DeactivateCollision()
{
	bIsCollisionEnabeld = false;
}

// Trace로 변경
//void UEnemyWeaponCollisionComponent::OnColliderBeginOverlap(UPrimitiveComponent* OverlappedComponent,
//	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
//	const FHitResult& SweepResult)
//{
//	if (!OtherActor || OtherActor == GetOwner()) return;
//	if (!CanHitActor(OtherActor)) { return; }
//
//	AlreadyHitActors.Add(OtherActor);
//
//	// TO-DO
//	/* 데미지 처리 **/
//	int a = 0;
//}

bool UEnemyWeaponCollisionComponent::CanHitActor(AActor* HitActor)
{
	return AlreadyHitActors.Contains(HitActor) == false;
}

void UEnemyWeaponCollisionComponent::SetWeaponMesh(UPrimitiveComponent* MeshComponent)
{
	WeaponMesh = MeshComponent;
}

void UEnemyWeaponCollisionComponent::AddIgnoredActor(AActor* Actor)
{
	IgnoredActors.Add(Actor);
}

void UEnemyWeaponCollisionComponent::RemoveIgnoredActor(AActor* Actor)
{
	IgnoredActors.Remove(Actor);
}