#include "Player/Abilities/GA_Player_SpecialHitReact.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Kismet/GameplayStatics.h"

#include "Player/BasePlayer.h"
#include "Player/Data/PlayerTags.h"

void UGA_Player_SpecialHitReact::CommitExecute(const FGameplayAbilitySpecHandle Handle,
										const FGameplayAbilityActorInfo* ActorInfo,
										const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo && ActorInfo->OwnerActor.IsValid())
	{
		UWorld* World = ActorInfo->OwnerActor->GetWorld();
		if (World && VoiceSound)
		{
			UGameplayStatics::PlaySound2D(World, VoiceSound);
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

	if (EventData.EventTag == PlayerTags::Event_Player_Grabbed)
		OnWraithBoss();
	else if (EventData.EventTag == PlayerTags::Event_Player_GrabTrigger)
		OnRampage();
	else if (EventData.EventTag == PlayerTags::Event_Player_GrabAniStart)
		OnPlayerGrapMontage();
}

void UGA_Player_SpecialHitReact::EndAbility(const FGameplayAbilitySpecHandle Handle,
									 const FGameplayAbilityActorInfo* ActorInfo,
									 const FGameplayAbilityActivationInfo ActivationInfo,
									 bool bReplicateEndAbility,
									 bool bWasCancelled)
{
	if (UAnimInstance* AnimInst = OwnerPlayer->GetMesh()->GetAnimInstance())
	{
		AnimInst->OnPlayMontageNotifyBegin.RemoveDynamic(this, &UGA_Player_SpecialHitReact::OnBeginNotify);
		AnimInst->OnPlayMontageNotifyEnd.RemoveDynamic(this, &UGA_Player_SpecialHitReact::OnEndNotify);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Player_SpecialHitReact::OnWraithBoss()
{
	if (UAnimInstance* AnimInst = OwnerPlayer->GetMesh()->GetAnimInstance())
	{
		AnimInst->OnPlayMontageNotifyBegin.AddDynamic(this, &UGA_Player_SpecialHitReact::OnBeginNotify);
		AnimInst->OnPlayMontageNotifyEnd.AddDynamic(this, &UGA_Player_SpecialHitReact::OnEndNotify);

		UAbilityTask_PlayMontageAndWait* Task =
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this,           // Ability 자신
				NAME_None,      // Task Instance Name
				WraithGrapMontage,  // 재생할 몽타주
				MontagePlayRate,           // 재생 속도
				NAME_None,      // Section Name (원하면 섹션 지정)
				false,          // Stop when ability ends
				1.0f            // Root Motion Scale
			);

		Task->OnCompleted.AddDynamic(this, &UGA_Player_SpecialHitReact::K2_EndAbility);
		Task->OnBlendOut.AddDynamic(this, &UGA_Player_SpecialHitReact::K2_EndAbility);
		Task->OnInterrupted.AddDynamic(this, &UGA_Player_SpecialHitReact::K2_EndAbility);
		Task->OnCancelled.AddDynamic(this, &UGA_Player_SpecialHitReact::K2_EndAbility);
		Task->ReadyForActivation();
	}
}

void UGA_Player_SpecialHitReact::OnRampage()
{
}

void UGA_Player_SpecialHitReact::OnPlayerGrapMontage()
{
	if (UAnimInstance* AnimInst = OwnerPlayer->GetMesh()->GetAnimInstance())
	{
		AnimInst->OnPlayMontageNotifyBegin.AddDynamic(this, &UGA_Player_SpecialHitReact::OnBeginNotify);
		AnimInst->OnPlayMontageNotifyEnd.AddDynamic(this, &UGA_Player_SpecialHitReact::OnEndNotify);

		UAbilityTask_PlayMontageAndWait* Task =
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this,           // Ability 자신
				NAME_None,      // Task Instance Name
				RampageGrapMontage,  // 재생할 몽타주
				MontagePlayRate,           // 재생 속도
				NAME_None,      // Section Name (원하면 섹션 지정)
				false,          // Stop when ability ends
				1.0f            // Root Motion Scale
			);

		Task->OnCompleted.AddDynamic(this, &UGA_Player_SpecialHitReact::K2_EndAbility);
		Task->OnBlendOut.AddDynamic(this, &UGA_Player_SpecialHitReact::K2_EndAbility);
		Task->OnInterrupted.AddDynamic(this, &UGA_Player_SpecialHitReact::K2_EndAbility);
		Task->OnCancelled.AddDynamic(this, &UGA_Player_SpecialHitReact::K2_EndAbility);
		Task->ReadyForActivation();
	}
}

void UGA_Player_SpecialHitReact::OnBeginNotify(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
	if (NotifyName == EndGrabNotify)
	{
		// Attach
		if (auto GrabMesh = Cast<UStaticMeshComponent>(EventData.OptionalObject.Get()))
		{
			const FAttachmentTransformRules rule = FAttachmentTransformRules(
				EAttachmentRule::SnapToTarget,
				EAttachmentRule::KeepWorld,
				EAttachmentRule::KeepWorld, true);

			OwnerPlayer->AttachToComponent(const_cast<UStaticMeshComponent*>(GrabMesh), rule, WraithSocketName);

			OwnerPlayer->GetCharacterMovement()->DisableMovement();
		}
	}
}

void UGA_Player_SpecialHitReact::OnEndNotify(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
	if (NotifyName == EndGrabNotify)
	{
		OwnerPlayer->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		OwnerPlayer->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	}
}
