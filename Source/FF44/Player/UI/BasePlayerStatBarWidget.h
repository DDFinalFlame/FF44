#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BasePlayerStatBarWidget.generated.h"

class UProgressBar;

UCLASS()
class FF44_API UBasePlayerStatBarWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	class UBorder* BackgroundBorder;

	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	class UProgressBar* StatBar;

	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	class UImage* StatOutline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatBar")
	FLinearColor FillColorAndOpacity = FLinearColor::White;

public:
	virtual void NativePreConstruct() override;

	void SetRatio(float Ratio);
};
