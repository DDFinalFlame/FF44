// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InventorySystem/InventorySystemInterface.h"

#include "CoreMinimal.h"
#include "Interactable/FF44InteractableActor.h"
#include "GameInstance/FF44GameInstance.h"
#include "FF44LootContainer.generated.h"

class UDataTable;

UCLASS()
class FF44_API AFF44LootContainer : public AFF44InteractableActor, public IInventorySystemInterface
{
    GENERATED_BODY()

public:
    AFF44LootContainer();
    virtual void BeginPlay() override;

    virtual UInventoryComponent* GetInventoryComponent() const override { return InventoryComponent; }
    virtual bool CanInteract_Implementation(AActor* Interactor) const override;
    virtual void Interact_Implementation(AActor* Interactor) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
    TArray<FItemInstance> LootItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
    bool bOneTimeOpen = false;

    UPROPERTY(BlueprintReadOnly, Category = "Loot")
    bool bOpened = false;

protected:
    UPROPERTY(EditAnywhere)
    UInventoryComponent* InventoryComponent;

    UPROPERTY(EditAnywhere, Category = "Data")
    UDataTable* ItemTable = nullptr;

public:

    UFUNCTION(BlueprintCallable, Category = "Loot")
    bool TakeItemToStash(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "Loot")
    void TakeAllToStash();

    UFUNCTION(BlueprintCallable, Category = "Loot")
    const TArray<FItemInstance>& GetLootItems() const { return LootItems; }
};