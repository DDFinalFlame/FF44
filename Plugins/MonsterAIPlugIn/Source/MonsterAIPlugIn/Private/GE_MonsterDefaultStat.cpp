// Fill out your copyright notice in the Description page of Project Settings.


#include "GE_MonsterDefaultStat.h"
#include "MonsterAttributeSet.h"

UGE_MonsterDefaultStat::UGE_MonsterDefaultStat()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo HealthModifier;
	HealthModifier.Attribute = UMonsterAttributeSet::GetHealthAttribute();
	HealthModifier.ModifierOp = EGameplayModOp::Additive;
	HealthModifier.ModifierMagnitude = FScalableFloat(100.f);

	Modifiers.Add(HealthModifier);

	FGameplayModifierInfo AttackModifier;
	AttackModifier.Attribute = UMonsterAttributeSet::GetAttackPowerAttribute();
	AttackModifier.ModifierOp = EGameplayModOp::Additive;
	AttackModifier.ModifierMagnitude = FScalableFloat(20.f);

	Modifiers.Add(AttackModifier);
}