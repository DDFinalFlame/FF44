#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FF44Interactable.h"
#include "Components/WidgetComponent.h"
#include "FF44InteractableActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UUserWidget;

UCLASS()
class FF44_API AFF44InteractableActor : public AActor, public IFF44Interactable
{
    GENERATED_BODY()
public:
    AFF44InteractableActor();

protected:
    virtual void BeginPlay() override;
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

    virtual bool CanInteract_Implementation(AActor* Interactor) const override;
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void OnFocus_Implementation(AActor* Interactor) override;
    virtual void OnUnfocus_Implementation(AActor* Interactor) override;

    UFUNCTION(BlueprintCallable, Category = "Interactable|UI")
    void SetPromptVisible(bool bVisible);

    void UpdatePromptFacing();

private:
    UFUNCTION()
    void OnRangeBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& Sweep);

    UFUNCTION()
    void OnRangeEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    TWeakObjectPtr<AActor> CurrentInteractor;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|UI")
    float PromptDistance = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|UI")
    float PromptHeight = 80.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|UI")
    bool bYawOnly = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|Physics")
    bool bUseGravity = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable|Physics")
    float InitialPhysicsSimTime = 0.0f;
};
