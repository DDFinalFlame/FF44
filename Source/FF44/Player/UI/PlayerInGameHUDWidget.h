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

	UPROPERTY()
	TWeakObjectPtr<const class UBasePlayerAttributeSet> PlayerAttrSet;

	float MaxTime = 0.f;
	float CurrentTime = 0.f;
	bool IsProgress = false;

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	virtual void InitASC(UAbilitySystemComponent* _OwnerASC, const UAttributeSet* _OwnerAttrSet) override;

	FDelegateHandle CurrentHPChangedDelegateHandle;
	FDelegateHandle CurrentStaminaChangedDelegateHandle;

	void SetProgressBar(float _MaxTime);
	void EndProgressBar();

protected:
	void OnCurrentHPChanged(const FOnAttributeChangeData& _Data);
	void OnCurrentStaminaChanged(const FOnAttributeChangeData& _Data);
};
