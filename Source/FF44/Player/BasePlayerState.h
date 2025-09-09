#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "BasePlayerState.generated.h"

class UAbilitySystemComponent;
class UInventoryComponent;

UCLASS()
class FF44_API ABasePlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	ABasePlayerState();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystemComponent")
	UAbilitySystemComponent* ASC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "InventoryComponent")
	UInventoryComponent* IC;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return ASC; }
	virtual UInventoryComponent* GetInventoryComponent() const { return IC; }
};
