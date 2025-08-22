#include "EC_DamageFromHit.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "GameplayEffect.h"
#include "GameplayTagsManager.h"
#include "MonsterAttributeSet.h"

// Ä¸Ã³ Á¤ÀÇ(Å¸°ÙÀÇ Defense¸¦ ½º³À¼¦ X, ¸Å Æ½ °è»ê O)
void UEC_DamageFromHit::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	// MonsterÀÇ spec?
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
		PlayerAttackCaptureAtt, EvaluationParameters, LocalMonsterAttackPower);

	float LocalPlayerDefence = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		MonsterDefenceCaptureDef, EvaluationParameters, LocalPlayerDefence);

	auto NewAttackPower = LocalMonsterAttackPower - LocalPlayerDefence;

	// Set new HP
	OutExecutionOutput.AddOutputModifier((FGameplayModifierEvaluatedData(
		UMonsterAttributeSet::GetHealthAttribute(), EGameplayModOp::Additive,
		-NewAttackPower)));

}
