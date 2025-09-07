#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BossHPBarWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class MONSTERAIPLUGIN_API UBossHPBarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void InitBossName(const FText& InBossName);

    UFUNCTION(BlueprintCallable)
    void UpdateHP(float Current, float Max);

protected:
    // UMG �����̳ʿ��� ProgressBar �̸��� HPProgressBar �� ���߸� �ڵ� ���ε��˴ϴ�.
    UPROPERTY(meta = (BindWidget))
    UProgressBar* HPProgressBar;

    // ����: ���� �̸� �ؽ�Ʈ���(�����̳ʿ��� BossNameText �� ����� ���ε�)
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* BossNameText;
};