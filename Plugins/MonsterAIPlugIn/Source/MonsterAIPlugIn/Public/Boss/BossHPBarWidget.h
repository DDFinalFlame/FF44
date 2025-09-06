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
    // UMG 디자이너에서 ProgressBar 이름을 HPProgressBar 로 맞추면 자동 바인딩됩니다.
    UPROPERTY(meta = (BindWidget))
    UProgressBar* HPProgressBar;

    // 선택: 보스 이름 텍스트블록(디자이너에서 BossNameText 로 만들면 바인딩)
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* BossNameText;
};