// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FF44Interactable.generated.h"

UINTERFACE(BlueprintType)
class UFF44Interactable : public UInterface
{
    GENERATED_BODY()
};

class IFF44Interactable
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    bool CanInteract(AActor* Interactor) const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    void Interact(AActor* Interactor);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    void OnFocus(AActor* Interactor);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    void OnUnfocus(AActor* Interactor);
};
