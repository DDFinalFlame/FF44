#include "MMC_AttackToDamage_.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "MonsterAttributeSet.h"
#include "NativeGameplayTags.h"


float UMMC_AttackToDamage_::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
    UE_LOG(LogTemp, Warning, TEXT("[MMC] ENTER"));

    // 1) SetByCaller �켱 (���� �������� Spec.Data->SetSetByCallerMagnitude("Data.AttackPower", FinalAttack)�� �־��)
    const FGameplayTag Tag_Attack = FGameplayTag::RequestGameplayTag(TEXT("Data.AttackPower"));
    float Attack = Spec.GetSetByCallerMagnitude(Tag_Attack, /*WarnIfNotFound=*/false, /*DefaultIfNotFound=*/-FLT_MAX);

    // 2) SetByCaller�� ������ �ҽ� ASC�� ���� AttackPower�� ����
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

    // 3) (����) ���⼭ ��ų ���/���� ���� �߰��� ���ص� ��
    // const float SkillCoeff = 1.0f;
    // Attack *= SkillCoeff;

    // 4) ��ȯ: GE Modifier������ Op �������� ��ȣ�� ���߼���.
    //  - Mod Op�� Additive�̰� Health�� ���� �Ÿ�, GE���� '����'�� �ְų� ���⼭ ������ ��ȯ�ϸ� �˴ϴ�.
    //  - ���� ���� GE Modifier���� Health�� Additive, Magnitude = �� MMC, �׸��� ���⼭ '����'�� ����� ��ȯ�մϴ�.
    return Attack;
}