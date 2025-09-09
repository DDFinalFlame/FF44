#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NextLevelWidget.generated.h"

class APlayerController;

UCLASS()
class FF44_API UNextLevelWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "DataSet")
	void DataToss();
};
