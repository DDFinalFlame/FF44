#pragma once

#include "CoreMinimal.h"
#include "Player/UI/BasePlayerHUDWidget.h"
#include "BaseMonsterHUDWidget.generated.h"

struct FOnAttributeChangeData;

UCLASS()
class FF44_API UBaseMonsterHUDWidget : public UBasePlayerHUDWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	class UBasePlayerStatBarWidget* HPBarWidget;

	UPROPERTY()
	TWeakObjectPtr<const class UMonsterAttributeSet> MonsterAttrSet;

public:
	virtual void NativeConstruct() override;

	virtual void InitASC(UAbilitySystemComponent* _OwnerASC, const UAttributeSet* _OwnerAttrSet) override;
	FDelegateHandle CurrentHPChangedDelegateHandle;

protected:
	void OnCurrentHPChanged(const FOnAttributeChangeData& _Data);
};
