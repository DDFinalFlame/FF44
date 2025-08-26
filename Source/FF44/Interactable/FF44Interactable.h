// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FF44Interactable.generated.h"

UINTERFACE(Blueprintable)
class UFF44Interactable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class FF44_API IFF44Interactable
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact")
    bool CanInteract(AActor* Interactor) const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact")
    void Interact(AActor* Interactor);

};
