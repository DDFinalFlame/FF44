#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

class UCanvasPanel;
class UBorder;
class UBackgroundBlur;
class UInventoryGridWidget;
class UButton;

UCLASS()
class FF44_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	UCanvasPanel* Canvas;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	UButton* EscapeButton;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	UInventoryGridWidget* PlayerGrid;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	UInventoryGridWidget* OtherGrid;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "OtherActor")
	AActor* OtherActor;

public:
	void SetInteractActor(AActor* _InterAct) { OtherActor = _InterAct; }
	UButton* GetEscapeButton() const { return EscapeButton; }

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UFUNCTION() 
	void VisibilityChanged(ESlateVisibility NewVis);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void DrawItemWigets();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void DrawOtherGrid();
};
