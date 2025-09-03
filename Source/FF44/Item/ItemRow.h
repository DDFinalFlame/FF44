#pragma once

#include "Engine/DataTable.h"
#include "ItemRow.generated.h"

USTRUCT(BlueprintType)
struct FItemRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Dimension;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* Icon;

	// �� ���� ��
	int32 ItemIndex;
};