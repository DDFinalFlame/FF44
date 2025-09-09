// �÷��̾��� �������� ������ ���ظ� �ִ� GE ���� Ŭ����

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
