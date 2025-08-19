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
			// ���ٸ� Combo_1 ��Ÿ�ָ� ���
            UAbilityTask_PlayMontageAndWait* Task =
                UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
                    this,           // Ability �ڽ�
                    NAME_None,      // Task Instance Name
                    AttackMontage,  // ����� ��Ÿ��
                    1.0f,           // ��� �ӵ�
                    NAME_None,      // Section Name (���ϸ� ���� ����)
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