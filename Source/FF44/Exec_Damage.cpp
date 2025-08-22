// Fill out your copyright notice in the Description page of Project Settings.


#include "Exec_Damage.h"
#include "AbilitySystemComponent.h"
#include "MonsterAttributeSet.h"
#include "Player/BasePlayerAttributeSet.h"

static float ReadAttack(UAbilitySystemComponent* ASC)
{
    if (!ASC) return 0.f;
    if (const UMonsterAttributeSet* M = ASC->GetSet<UMonsterAttributeSet>())
        return M->GetAttackPower();
    if (const UBasePlayerAttributeSet* P = ASC->GetSet<UBasePlayerAttributeSet>())
        return P->GetAttackPower();
    return 0.f;
}

static float ReadDefense(UAbilitySystemComponent* ASC)
{
    if (!ASC) return 0.f;
    if (const UMonsterAttributeSet* M = ASC->GetSet<UMonsterAttributeSet>())
        return M->GetHealth();
    if (const UBasePlayerAttributeSet* P = ASC->GetSet<UBasePlayerAttributeSet>())
        return P->GetHealth();
    return 0.f;
}

UExec_Damage::UExec_Damage()
{
 
}

void UExec_Damage::Execute_Implementation(
    const FGameplayEffectCustomExecutionParameters& Params,
    FGameplayEffectCustomExecutionOutput& Out) const
{
    const FGameplayEffectSpec& Spec = Params.GetOwningSpec();

    // 1) 기본값: 소스/타깃 ASC에서 직접 읽음
    UAbilitySystemComponent* SrcASC = Params.GetSourceAbilitySystemComponent();
    UAbilitySystemComponent* TgtASC = Params.GetTargetAbilitySystemComponent();

    float Attack = ReadAttack(SrcASC);
    float Defense = ReadDefense(TgtASC);

    //// 2) (선택) GA에서 SetByCaller로 싣고 온 값이 있으면 우선 사용
    ////    ex) Player.Stat.AttackPower / Target.Stat.Defense 등
    //Attack = Spec.GetSetByCallerMagnitude(
    //    FGameplayTag::RequestGameplayTag(TEXT("Player.Stat.AttackPower"), false),
    //    false, Attack);

    //Defense = Spec.GetSetByCallerMagnitude(
    //    FGameplayTag::RequestGameplayTag(TEXT("Target.Stat.Defense"), false),
    //    false, Defense);

    // 3) 최종 데미지 계산
    float Damage = FMath::Max(Attack - Defense, 1.f);

    // 4) Health 감소 적용
    Out.AddOutputModifier(FGameplayModifierEvaluatedData(
        UMonsterAttributeSet::GetHealthAttribute(),
        EGameplayModOp::Additive,
        -Damage
    ));
}