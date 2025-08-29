// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FF44Interactable.h"
#include "FF44InteractableActor.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UWidgetComponent;
class UUserWidget;

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

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactable")
    UStaticMeshComponent* Mesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactable")
    USphereComponent* InteractRange;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactable|UI")
    UWidgetComponent* PromptWidget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|UI")
    TSubclassOf<UUserWidget> PromptWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|UI")
    float PromptDistance = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|UI")
    float PromptHeight = 80.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|UI")
    bool bYawOnly = true;

public:
    UFUNCTION(BlueprintImplementableEvent, Category = "Interactable")
    void BP_OnFocus(AActor* Interactor);

    UFUNCTION(BlueprintImplementableEvent, Category = "Interactable")
    void BP_OnUnfocus(AActor* Interactor);

    UFUNCTION()
    void OnRangeBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

    UFUNCTION()
    void OnRangeEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    virtual bool CanInteract_Implementation(AActor* Interactor) const override;
    virtual void Interact_Implementation(AActor* Interactor) override;

    UFUNCTION(BlueprintCallable, Category = "Interactable|UI")
    void SetPromptVisible(bool bVisible);

    void UpdatePromptFacing();

private:
    TWeakObjectPtr<AActor> CurrentInteractor;

};
