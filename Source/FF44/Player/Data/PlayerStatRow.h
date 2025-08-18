#pragma once
#include "Engine/DataTable.h"
#include "PlayerStatRow.generated.h"

USTRUCT(BlueprintType)
struct FPlayerStatRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxHealth = 100.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxStamina = 100.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackPower = 10.f;
};