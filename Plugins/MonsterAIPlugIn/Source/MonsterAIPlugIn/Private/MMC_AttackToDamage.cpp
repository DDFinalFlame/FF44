// Fill out your copyright notice in the Description page of Project Settings.


#include "MMC_AttackToDamage.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectTypes.h"
#include "AttackStatProvider.h"



float UMMC_AttackToDamage::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
    const FGameplayTag TagAttack = FGameplayTag::RequestGameplayTag(TEXT("Player.Stat.AttackPower"));
    const FGameplayTag TagDefense = FGameplayTag::RequestGameplayTag(TEXT("Player.Stat.Defense"));
    const FGameplayTag TagSpeed = FGameplayTag::RequestGameplayTag(TEXT("Data.MoveSpeed"));
    float Attack = Spec.GetSetByCallerMagnitude(TagAttack, false, -FLT_MAX);
    float Defense = Spec.GetSetByCallerMagnitude(TagDefense, false, -FLT_MAX);
    float Speed = Spec.GetSetByCallerMagnitude(TagSpeed, false, -FLT_MAX);

    float FinalAttack = Attack + Defense - Speed ;

    if (Attack == -FLT_MAX) {
        const FGameplayTag Old = FGameplayTag::RequestGameplayTag(TEXT("Data.AttackPower"));
        Attack = Spec.GetSetByCallerMagnitude(Old, false, 0.f);
    }
    return -FinalAttack;
}
