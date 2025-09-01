#include "Player/Effects/GEEC_PlayerDamaged.h"
#include "Player/BasePlayerAttributeSet.h"
#include "AbilitySystemComponent.h"

void UGEEC_PlayerDamaged::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
												 OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	// Monster¿« spec?
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags =
		Spec.CapturedSourceTags.GetAggregatedTags();

	const FGameplayTagContainer* TargetTags =
		Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	float LocalMonsterAttackPower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		MonsterAttackCaptureDef, EvaluationParameters, LocalMonsterAttackPower);

	float LocalRockPower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		RockCaptureDef, EvaluationParameters, LocalRockPower);

	float LocalPlayerDefence = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		PlayerDefenceCaptureDef, EvaluationParameters, LocalPlayerDefence);

	auto NewAttackPower = LocalRockPower + LocalMonsterAttackPower - LocalPlayerDefence;

	// Set new HP
	OutExecutionOutput.AddOutputModifier((FGameplayModifierEvaluatedData(
		UBasePlayerAttributeSet::GetCurrentHPAttribute(), EGameplayModOp::Additive,
		-NewAttackPower)));

	////////////////////////////////////////////////////////////////////////////////

	const FGameplayEffectContextHandle& Context = Spec.GetContext();

	UAbilitySystemComponent* SourceASC = Spec.GetContext().GetOriginalInstigatorAbilitySystemComponent();
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

}
