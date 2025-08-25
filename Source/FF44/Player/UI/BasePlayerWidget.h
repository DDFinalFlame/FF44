#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BasePlayerWidget.generated.h"

class UAbilitySystemComponent;
class UBasePlayerAttributeSet;

UCLASS()
class FF44_API UBasePlayerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Init")
	virtual void InitASC(UAbilitySystemComponent* _OwnerASC, const UBasePlayerAttributeSet* _OwnerAttrSet);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ASC")
	TObjectPtr<UAbilitySystemComponent> OwnerASC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ASC")
	TObjectPtr<const UBasePlayerAttributeSet> OwnerAttrSet;

};
