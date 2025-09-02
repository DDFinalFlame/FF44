#pragma once
#include "Engine/DataTable.h"
#include "MonsterStatRow.generated.h"

USTRUCT(BlueprintType)
struct FMonsterStatRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) 
    float MaxHealth = 100.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackPower = 10.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) 
    float MoveSpeed = 300.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Defense = 5.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) 
    float DetectDistance = 1500.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) 
    float AttackDistance = 800.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) 
    int32 Exp = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) 
    int32 GoldMin = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) 
    int32 GoldMax = 0;
};