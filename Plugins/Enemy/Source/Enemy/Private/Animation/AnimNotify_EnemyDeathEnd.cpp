// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify_EnemyDeathEnd.h"

#include "BaseEnemy.h"

void UAnimNotify_EnemyDeathEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;

    // Dissolve Ω√¿€
    if (ABaseEnemy* Enemy = Cast<ABaseEnemy>(MeshComp->GetOwner()))
    {
        Enemy->StartDissolve();
    }

    //// Montage ∏ÿ√ﬂ±‚
    //if (UAnimInstance* AnimInst = MeshComp->GetAnimInstance())
    //{
    //    if (UAnimMontage* CurrentMontage = AnimInst->GetCurrentActiveMontage())
    //    {
    //        AnimInst->Montage_Stop(0.0f, CurrentMontage); // ¡ÔΩ√ ∏ÿ√„
    //    }

    //}
}
