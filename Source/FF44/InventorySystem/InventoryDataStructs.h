#pragma once
#include "InventoryDataStructs.generated.h"

USTRUCT()
struct FLines {
	GENERATED_USTRUCT_BODY();

	TArray<FVector2D> XLines;
	TArray<FVector2D> YLines;
};

USTRUCT()
struct FMousePositionInTile {
	GENERATED_USTRUCT_BODY();

	bool Right;
	bool Down;
};