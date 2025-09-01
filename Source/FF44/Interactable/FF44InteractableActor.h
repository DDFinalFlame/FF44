// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FF44Interactable.h"
#include "FF44InteractableActor.generated.h"

class UStaticMeshComponent;
class USphereComponent;

UCLASS()
class FF44_API AFF44InteractableActor : public AActor, public IFF44Interactable
{
	GENERATED_BODY()
	
public:	
	AFF44InteractableActor();

protected:
	virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactable")
    UStaticMeshComponent* Mesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactable")
    USphereComponent* InteractRange;

    virtual bool CanInteract_Implementation(AActor* Interactor) const override;
    virtual void Interact_Implementation(AActor* Interactor) override;

};
