#pragma once
#include "Engine/DataAsset.h" 
#include "GameplayTagContainer.h"
#include "PlayerDefinition.generated.h"

UCLASS(BlueprintType)
class FF44_API UPlayerDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	UDataTable* PlayerMetaDataTable = nullptr;
};