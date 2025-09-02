// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MMC_AttackToDamage.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectTypes.h"
#include "Interface/AttackStatProvider.h"



float UMMC_AttackToDamage::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
    UE_LOG(LogTemp, Warning, TEXT("[MMC] ENTER"));

    const FGameplayEffectContextHandle& Ctx = Spec.GetContext();
    UAbilitySystemComponent* SrcASC = Ctx.GetInstigatorAbilitySystemComponent();
    AActor* Source = SrcASC ? SrcASC->GetAvatarActor() : Ctx.GetOriginalInstigator();

    float Attack = 0.f;
    if (Source && Source->GetClass()->ImplementsInterface(UAttackStatProvider::StaticClass()))
        Attack = IAttackStatProvider::Execute_GetAttackPower(Source);

    //Attack *= (-1);
    return Attack;
}
