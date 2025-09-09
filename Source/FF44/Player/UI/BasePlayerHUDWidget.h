#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BasePlayerHUDWidget.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;

UCLASS()
class FF44_API UBasePlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Init")
	virtual void InitASC(UAbilitySystemComponent* _OwnerASC, const UAttributeSet* _OwnerAttrSet);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ASC")
	TWeakObjectPtr<UAbilitySystemComponent> OwnerASC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ASC")
	TWeakObjectPtr<const UAttributeSet> OwnerAttrSet;

};
