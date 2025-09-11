// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/FF44LootContainer.h"
#include "GameInstance/FF44GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h"

#include "Player/BasePlayer.h"
#include "Player/BasePlayerController.h"
#include "InventorySystem/InventoryComponent.h"
#include "InventorySystem/Widget/InventoryWidget.h"

AFF44LootContainer::AFF44LootContainer()
{
    InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
}

void AFF44LootContainer::BeginPlay()
{
    AFF44InteractableActor::BeginPlay();

    if (!ItemTable) return;

    TArray<FName> Names = ItemTable->GetRowNames();
    for (auto itemName : Names)
    {
        if (auto itemPCS = ItemData.Find(itemName)) {
            for (int32 i = 0; i < *itemPCS; i++) {
                FItemRow* row = ItemTable->FindRow<FItemRow>(itemName, FString("Get Item"));
                FItemRow* item = new FItemRow(*row);
                InventoryComponent->TryAddItem(item);
            }
        }
    }
}

bool AFF44LootContainer::CanInteract_Implementation(AActor* Interactor) const
{
    if (bOneTimeOpen && bOpened) return false;

    return true;
}

void AFF44LootContainer::Interact_Implementation(AActor* Interactor)
{
    if (!CanInteract_Implementation(Interactor)) return;

    bOpened = true;

    SetPromptVisible(false);

    if (auto player = Cast<ABasePlayer>(Interactor))
    {
        auto controller = player->GetBasePlayerController();
        controller->GetInventoryWidget()->SetInteractActor(this);
        controller->ToggleInventory();
    }
}

bool AFF44LootContainer::TakeItemToStash(int32 Index)
{
    if (!LootItems.IsValidIndex(Index)) return false;

    UWorld* World = GetWorld();
    if (!World) return false;

    if (UGameInstance* GI = UGameplayStatics::GetGameInstance(World))
    {
        if (UFF44GameInstance* MyGI = Cast<UFF44GameInstance>(GI))
        {
            MyGI->AddItem(LootItems[Index]);
            LootItems.RemoveAt(Index);
            return true;
        }
    }
    return false;
}

void AFF44LootContainer::TakeAllToStash()
{
    UWorld* World = GetWorld();
    if (!World || LootItems.Num() == 0) return;

    if (UGameInstance* GI = UGameplayStatics::GetGameInstance(World))
    {
        if (UFF44GameInstance* MyGI = Cast<UFF44GameInstance>(GI))
        {
            for (const FItemInstance& It : LootItems)
            {
                MyGI->AddItem(It);
            }
            LootItems.Empty();
        }
    }
}


