// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss/BossHPBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UBossHPBarWidget::InitBossName(const FText& InBossName)
{
    if (BossNameText)
    {
        BossNameText->SetText(InBossName);
    }
}

void UBossHPBarWidget::UpdateHP(float Current, float Max)
{
    if (HPProgressBar && Max > 0.f)
    {
        const float Pct = FMath::Clamp(Current / Max, 0.f, 1.f);
        HPProgressBar->SetPercent(Pct);
    }
}
