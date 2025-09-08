#include "Player/Abilities/GA_Player_SpecialHitReact.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Kismet/GameplayStatics.h"
#include "MotionWarpingComponent.h"

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

// Motion Warping을 사용한 Animation
// Mesh Component를 따라 가도록 AddOrUpdateWarpTargetFromComponent 사용
// 주요 설정은 다음과 같음
// 1. SetMovementMode : 시작시 Fly, 마치고 Walk로 변경
// 2. 
void UGA_Player_SpecialHitReact::OnWraithBoss()
{
	if (UAnimInstance* AnimInst = OwnerPlayer->GetMesh()->GetAnimInstance())
	{
		if (auto GrabMesh = Cast<UStaticMeshComponent>(EventData.OptionalObject.Get()))
		{
			if (!GrabMesh->DoesSocketExist(WraithSocketName))
			{
				UE_LOG(LogTemp, Warning, TEXT("Socket %s not found on %s"), *WraithSocketName.ToString(), *GrabMesh->GetName());
				return;
			}

			OwnerPlayer->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
			OwnerPlayer->GetMotionWarpingComponent()->AddOrUpdateWarpTargetFromComponent(
				GrabMotionWarpingNotify,
				GrabMesh,
				WraithSocketName,
				true,
				FVector::ZeroVector,
				FRotator::ZeroRotator
			);
		}

		AnimInst->OnPlayMontageNotifyBegin.AddDynamic(this, &UGA_Player_SpecialHitReact::OnBeginNotify);
		AnimInst->OnPlayMontageNotifyEnd.AddDynamic(this, &UGA_Player_SpecialHitReact::OnEndNotify);

		UAbilityTask_PlayMontageAndWait* Task =
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this,          
				NAME_None,     
				WraithGrapMontage, 
				MontagePlayRate,   
				NAME_None,     
				false,         
				1.0f           
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
	OwnerPlayer->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

	UAbilityTask_PlayMontageAndWait* Task =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,          
			NAME_None,     
			HitMontage,  
			MontagePlayRate,
			NAME_None,     
			false,         
			1.0f           
		);

	Task->OnCompleted.AddDynamic(this, &UGA_Player_SpecialHitReact::K2_EndAbility);
	Task->OnBlendOut.AddDynamic(this, &UGA_Player_SpecialHitReact::K2_EndAbility);
	Task->OnInterrupted.AddDynamic(this, &UGA_Player_SpecialHitReact::K2_EndAbility);
	Task->OnCancelled.AddDynamic(this, &UGA_Player_SpecialHitReact::K2_EndAbility);
	Task->ReadyForActivation();
}

void UGA_Player_SpecialHitReact::OnPlayerGrapMontage()
{
	if (UAnimInstance* AnimInst = OwnerPlayer->GetMesh()->GetAnimInstance())
	{
		AnimInst->OnPlayMontageNotifyBegin.AddDynamic(this, &UGA_Player_SpecialHitReact::OnBeginNotify);
		AnimInst->OnPlayMontageNotifyEnd.AddDynamic(this, &UGA_Player_SpecialHitReact::OnEndNotify);

		UAbilityTask_PlayMontageAndWait* Task =
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this,
				NAME_None,
				RampageGrapMontage,
				MontagePlayRate,
				NAME_None, 
				false,  
				2.0f    
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
	if (NotifyName == EndThrowNotify)
	{
		USceneComponent* Child = OwnerPlayer->GetRootComponent();

		// 부모 회전 영향 차단
		Child->SetUsingAbsoluteRotation(true);

		if (auto boss = Cast<ACharacter>(EventData.Instigator.Get()))
		{			
			FAttachmentTransformRules rule =
				FAttachmentTransformRules(
					EAttachmentRule::SnapToTarget,
					EAttachmentRule::KeepWorld, 
					EAttachmentRule::KeepWorld, false);
			OwnerPlayer->AttachToComponent(boss->GetMesh(), rule, BossSocketName);
		}
	}
}

void UGA_Player_SpecialHitReact::OnEndNotify(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
	if (NotifyName == EndGrabNotify)
	{
		OwnerPlayer->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	}

	if (NotifyName == EndThrowNotify)
	{
		OwnerPlayer->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		OwnerPlayer->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
		USceneComponent* Child = OwnerPlayer->GetRootComponent();

		// 부모 회전 영향 차단
		Child->SetUsingAbsoluteRotation(false);
	}
}
