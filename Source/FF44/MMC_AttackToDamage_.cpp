#include "MMC_AttackToDamage_.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "MonsterAttributeSet.h"
#include "NativeGameplayTags.h"


float UMMC_AttackToDamage_::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
    UE_LOG(LogTemp, Warning, TEXT("[MMC] ENTER"));

    // 1) SetByCaller 우선 (공격 시점에서 Spec.Data->SetSetByCallerMagnitude("Data.AttackPower", FinalAttack)로 넣어둠)
    const FGameplayTag Tag_Attack = FGameplayTag::RequestGameplayTag(TEXT("Data.AttackPower"));
    float Attack = Spec.GetSetByCallerMagnitude(Tag_Attack, /*WarnIfNotFound=*/false, /*DefaultIfNotFound=*/-FLT_MAX);

    // 2) SetByCaller가 없으면 소스 ASC의 현재 AttackPower로 폴백
    if (Attack == -FLT_MAX)
    {
        const FGameplayEffectContextHandle& Ctx = Spec.GetContext();
        UAbilitySystemComponent* SrcASC = Ctx.GetInstigatorAbilitySystemComponent();
        if (SrcASC)
        {
            Attack = SrcASC->GetNumericAttribute(UMonsterAttributeSet::GetAttackPowerAttribute());
        }
        else
        {
            Attack = 0.f;
        }
    }

    // 3) (선택) 여기서 스킬 계수/버프 등을 추가로 곱해도 됨
    // const float SkillCoeff = 1.0f;
    // Attack *= SkillCoeff;

    // 4) 반환: GE Modifier에서의 Op 기준으로 부호를 맞추세요.
    //  - Mod Op가 Additive이고 Health를 깎을 거면, GE에서 '음수'로 넣거나 여기서 음수로 반환하면 됩니다.
    //  - 저는 보통 GE Modifier에서 Health에 Additive, Magnitude = 이 MMC, 그리고 여기서 '음수'로 만들어 반환합니다.
    return Attack;
}