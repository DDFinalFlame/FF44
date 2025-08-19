#include "Player/Abilities/GA_Player_Attack_Combo.h"

#include "Components/SphereComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameplayTagContainer.h"

#include "Player/BasePlayer.h"
#include "Weapon/BaseWeapon.h"

void UGA_Player_Attack_Combo::OnAttack_Implementation()
{
    if (UAnimInstance* AnimInst = OwnerPlayer->GetMesh()->GetAnimInstance())
    {
        AnimInst->OnPlayMontageNotifyBegin.AddDynamic(this, &UGA_Player_Attack_Combo::OnEnableAttack);
        AnimInst->OnPlayMontageNotifyEnd.AddDynamic(this, &UGA_Player_Attack_Combo::OnDisableAttack);

        if(USkeletalMeshComponent* Mesh = OwnerPlayer->GetMesh())
        {
			// 없다면 Combo_1 몽타주를 재생
            UAbilityTask_PlayMontageAndWait* Task =
                UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
                    this,           // Ability 자신
                    NAME_None,      // Task Instance Name
                    AttackMontage,  // 재생할 몽타주
                    1.0f,           // 재생 속도
                    NAME_None,      // Section Name (원하면 섹션 지정)
                    false,          // Stop when ability ends
                    1.0f            // Root Motion Scale
                );

            Task->OnCompleted.AddDynamic(this, &UGA_Player_Attack_Combo::OnMontageEnded);
            Task->OnCancelled.AddDynamic(this, &UGA_Player_Attack_Combo::OnMontageEnded);
            Task->OnInterrupted.AddDynamic(this, &UGA_Player_Attack_Combo::OnMontageEnded);
            Task->ReadyForActivation();
		}
    }
}

void UGA_Player_Attack_Combo::UnbindMontage()
{
    if (UAnimInstance* AnimInst = OwnerPlayer->GetMesh()->GetAnimInstance())
    {
        AnimInst->OnPlayMontageNotifyBegin.RemoveDynamic(this, &UGA_Player_Attack_Combo::OnEnableAttack);
        AnimInst->OnPlayMontageNotifyEnd.RemoveDynamic(this, &UGA_Player_Attack_Combo::OnDisableAttack);
    }

    GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(ComboEnabledTag);
}

void UGA_Player_Attack_Combo::OnEnableAttack(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
    if (NotifyName == CollisionNotifyName)
    {
        if (OwnerWeapon)
        {
            OwnerWeapon->GetWeaponCollision()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        }
    }

    if(NotifyName == ComboNotifyName)
    {
		GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(ComboEnabledTag);
	}
}

void UGA_Player_Attack_Combo::OnDisableAttack(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
    if (NotifyName == CollisionNotifyName)
    {
        if (OwnerWeapon)
        {
            OwnerWeapon->GetWeaponCollision()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }

    if(NotifyName == ComboNotifyName)
    {
        GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(ComboEnabledTag);
	}
}