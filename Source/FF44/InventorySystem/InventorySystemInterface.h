#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventorySystemInterface.generated.h"

class UInventoryComponent;

UINTERFACE(MinimalAPI)
class UInventorySystemInterface : public UInterface
{
	GENERATED_BODY()
};

class FF44_API IInventorySystemInterface
{
	GENERATED_BODY()

public:
	virtual UInventoryComponent* GetInventoryComponent() const = 0;
};
