#include "Player/Abilities/GA_Player_HitReact.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Kismet/GameplayStatics.h"

#include "Player/BasePlayer.h"
#include "Player/Data/PlayerTags.h"

UGA_Player_HitReact::UGA_Player_HitReact()
{
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(PlayerTags::Ability_Player_HitReact);
	SetAssetTags(AssetTags);

	CancelAbilitiesWithTag.AddTag(PlayerTags::Ability_Player_Attack);
	CancelAbilitiesWithTag.AddTag(PlayerTags::Ability_Player_Potion);

	BlockAbilitiesWithTag.AddTag(PlayerTags::Ability_Player_Attack);
	BlockAbilitiesWithTag.AddTag(PlayerTags::Ability_Player_Dodge);
	BlockAbilitiesWithTag.AddTag(PlayerTags::Ability_Player_Potion);

	ActivationOwnedTags.AddTag(PlayerTags::State_Player_HitReacting);

	ActivationBlockedTags.AddTag(PlayerTags::State_Player_Dead);
}

void UGA_Player_HitReact::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
									const FGameplayAbilityActorInfo* ActorInfo,
									const FGameplayAbilityActivationInfo ActivationInfo,
									const FGameplayEventData* TriggerEventData)
{
	if (ABasePlayer* player = Cast<ABasePlayer>(ActorInfo->AvatarActor.Get()))
	{
		OwnerPlayer = player;
		if (TriggerEventData)
			EventData = *TriggerEventData;
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
}

void UGA_Player_HitReact::CommitExecute(const FGameplayAbilitySpecHandle Handle,
										const FGameplayAbilityActorInfo* ActorInfo,
										const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo && ActorInfo->OwnerActor.IsValid())
	{
		UWorld* World = ActorInfo->OwnerActor->GetWorld();
		if (World && VoiceSound && ArmorSound)
		{
			UGameplayStatics::PlaySound2D(World, VoiceSound);
			UGameplayStatics::PlaySound2D(World, ArmorSound);
		}
		else
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* Task =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,           // Ability 자신
			NAME_None,      // Task Instance Name
			HitMontage,  // 재생할 몽타주
			MontagePlayRate,           // 재생 속도
			NAME_None,      // Section Name (원하면 섹션 지정)
			false,          // Stop when ability ends
			1.0f            // Root Motion Scale
		);

	const FVector  Loc = OwnerPlayer->GetActorLocation();
	const FRotator Rot = OwnerPlayer->GetActorRotation();

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PSystem, FTransform(Rot, Loc));

	Task->OnCompleted.AddDynamic(this, &UGA_Player_HitReact::K2_EndAbility);
	Task->OnBlendedIn.AddDynamic(this, &UGA_Player_HitReact::OnBlendInHitReact);
	Task->OnBlendOut.AddDynamic(this, &UGA_Player_HitReact::K2_EndAbility);
	Task->OnInterrupted.AddDynamic(this, &UGA_Player_HitReact::K2_EndAbility);
	Task->OnCancelled.AddDynamic(this, &UGA_Player_HitReact::K2_EndAbility);
	Task->ReadyForActivation();
}

void UGA_Player_HitReact::EndAbility(const FGameplayAbilitySpecHandle Handle,
							   const FGameplayAbilityActorInfo* ActorInfo, 
							   const FGameplayAbilityActivationInfo ActivationInfo, 
							   bool bReplicateEndAbility, 
							   bool bWasCancelled)
{


	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Player_HitReact::OnBlendInHitReact()
{
	// Ability를 가지고 있는지?
	if (EventData.Instigator) {
		if (UAbilitySystemComponent* ASC = EventData.Instigator->FindComponentByClass<UAbilitySystemComponent>())
		{
			// ASC 사용 가능
			FGameplayEffectSpecHandle specHandle = ASC->MakeOutgoingSpec(DamageEffectClass, 0.f, EventData.ContextHandle);
			FGameplayEffectSpec* spec = specHandle.Data.Get();

			OwnerPlayer->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*spec);
		}
	}
	else
	{
		auto ASC = OwnerPlayer->GetAbilitySystemComponent();
		FGameplayEffectSpecHandle specHandle = ASC->MakeOutgoingSpec(DamageEffectClass, 0.f, ASC->MakeEffectContext());
		FGameplayEffectSpec* spec = specHandle.Data.Get();
		ASC->ApplyGameplayEffectSpecToSelf(*spec);
	}

	const FVector  Loc = OwnerPlayer->GetActorLocation();
	const FRotator Rot = OwnerPlayer->GetActorRotation();

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PSystem, FTransform(Rot, Loc));
}

void UGA_Player_HitReact::OnBeginNotify(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
}

void UGA_Player_HitReact::OnEndNotify(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
}
