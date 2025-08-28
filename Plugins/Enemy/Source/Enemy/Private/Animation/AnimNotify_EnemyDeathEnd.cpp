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
}
