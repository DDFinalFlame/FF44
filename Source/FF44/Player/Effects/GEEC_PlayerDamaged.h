// 플레이어의 공격으로 적에게 피해를 주는 GE 연산 클래스

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "GEEC_PlayerDamaged.generated.h"

/**
 * 
 */
UCLASS()
class FF44_API UGEEC_PlayerDamaged : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayEffectAttributeCaptureDefinition MonsterAttackCaptureDef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayEffectAttributeCaptureDefinition RockCaptureDef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayEffectAttributeCaptureDefinition PlayerDefenceCaptureDef;

	void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, 
								OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
