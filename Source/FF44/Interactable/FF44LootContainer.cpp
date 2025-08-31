// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/FF44LootContainer.h"
#include "GameInstance/FF44GameInstance.h"
#include "Kismet/GameplayStatics.h"

AFF44LootContainer::AFF44LootContainer()
{

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

    BP_OpenLootUI(Interactor);
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


