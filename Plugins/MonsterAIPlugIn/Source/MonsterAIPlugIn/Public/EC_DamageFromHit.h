#pragma once
#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "EC_DamageFromHit.generated.h"

UCLASS()
class MONSTERAIPLUGIN_API UEC_DamageFromHit : public UGameplayEffectExecutionCalculation
{
    GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayEffectAttributeCaptureDefinition PlayerAttackCaptureAtt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayEffectAttributeCaptureDefinition MonsterDefenceCaptureDef;

	void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};