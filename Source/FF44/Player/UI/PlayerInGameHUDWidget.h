#pragma once

#include "CoreMinimal.h"
#include "Player/UI/BasePlayerHUDWidget.h"
#include "PlayerInGameHUDWidget.generated.h"

class UBasePlayerStatBarWidget;
struct FOnAttributeChangeData;

UCLASS()
class FF44_API UPlayerInGameHUDWidget : public UBasePlayerHUDWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	class UBasePlayerStatBarWidget* HPBarWidget;

	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	class UBasePlayerStatBarWidget* StaminaBarWidget;

	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	class UBasePlayerStatBarWidget* ProgressBarWidget;

	float MaxTime = 0.f;
	float CurrentTime = 0.f;
	bool IsProgress = false;

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	virtual void InitASC(UAbilitySystemComponent* _OwnerASC, const UBasePlayerAttributeSet* _OwnerAttrSet) override;

	FDelegateHandle CurrentHPChangedDelegateHandle;
	FDelegateHandle CurrentStaminaChangedDelegateHandle;

	void SetProgressBar(float _MaxTime);
	void EndProgressBar();

protected:
	void OnCurrentHPChanged(const FOnAttributeChangeData& _Data);
	void OnCurrentStaminaChanged(const FOnAttributeChangeData& _Data);
};
