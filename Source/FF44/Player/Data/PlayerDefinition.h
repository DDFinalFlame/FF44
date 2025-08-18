#pragma once
#include "Engine/DataAsset.h" 
#include "GameplayTagContainer.h"
#include "PlayerDefinition.generated.h"

UCLASS(BlueprintType)
class FF44_API UPlayerDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    // ���ڴ� DataTable���� ã�� RowName
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Name")
    FName StatRowName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Init")
    TSubclassOf<class UGameplayEffect> InitStatGE_SetByCaller;
};