// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Monster/MonsterCharacter.h"
#include "BossCharacter.generated.h"

class UBossPhaseComponent;
class UBossArenaComponent;
class UBossCutsceneComponent;
class UBossPartsComponent;
class UBossRewardComponent;

UCLASS()
class MONSTERAIPLUGIN_API ABossCharacter : public AMonsterCharacter
{
    GENERATED_BODY()
public:
    ABossCharacter();

protected:
    virtual void BeginPlay() override;

public:
    //UFUNCTION(BlueprintCallable)
    //void StartBossIntro();

    //UFUNCTION(BlueprintCallable)
    //void TryPhaseTransition(float CurrentHP, float MaxHP);

protected:
    //UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss")
    //UBossPhaseComponent* PhaseComp;

    //UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss")
    //UBossArenaComponent* ArenaComp;

    //UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss")
    //UBossCutsceneComponent* CutsceneComp;
	
};
