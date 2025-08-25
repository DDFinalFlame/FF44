#pragma once

#include "CoreMinimal.h"
#include "Player/UI/BasePlayerWidget.h"
#include "PlayerHPWidget.generated.h"

struct FOnAttributeChangeData;

UCLASS()
class FF44_API UPlayerHPWidget : public UBasePlayerWidget
{
	GENERATED_BODY()

public:
	virtual void InitASC(UAbilitySystemComponent* _OwnerASC, const UBasePlayerAttributeSet* _OwnerAttrSet) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "value")
	float CurrentHP = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "value")
	float MaxHP = 0.f;

private:
	FDelegateHandle CurrentHPChangedDelegateHandle;
	FDelegateHandle MaxHPChangedDelegateHandle;

	void OnCurrentHPChanged(const FOnAttributeChangeData& _Data);
	void OnMaxHPChanged(const FOnAttributeChangeData& _Data);

	void UpdateHPUI();
};
