// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FF44Interactable.h"
#include "FF44InteractableActor.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UWidgetComponent;

UCLASS()
class FF44_API AFF44InteractableActor : public AActor, public IFF44Interactable
{
    GENERATED_BODY()

public:
    AFF44InteractableActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaSeconds) override;

    virtual bool CanInteract_Implementation(AActor* Interactor) const override;
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void OnFocus_Implementation(AActor* Interactor) override;
    virtual void OnUnfocus_Implementation(AActor* Interactor) override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactable")
    UStaticMeshComponent* Mesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactable")
    USphereComponent* InteractRange;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactable|Prompt")
    USceneComponent* PromptAnchor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactable")
    UWidgetComponent* PromptWidget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
    TSubclassOf<UUserWidget> PromptWidgetClass;

    UPROPERTY()
    TWeakObjectPtr<AActor> CurrentInteractor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|Prompt")
    float PromptMidAlpha = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|Prompt")
    bool bKeepZFromAnchor = true;

    void SetPromptVisible(bool bVisible);
    void UpdatePromptFacing();

    UFUNCTION()
    void OnRangeBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& Sweep);

    UFUNCTION()
    void OnRangeEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
