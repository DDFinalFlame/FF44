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
class UAbilitySystemComponent;
class UGameplayEffect;


UCLASS()
class MONSTERAIPLUGIN_API ABossCharacter : public AMonsterCharacter
{
    GENERATED_BODY()
public:
    ABossCharacter();

protected:
    virtual void BeginPlay() override; 

    UPROPERTY(EditDefaultsOnly, Category = "Boss|GA")
    TSubclassOf<class UGameplayAbility> Phase1AbilityClass; // = UGA_BossPhase1

    bool bPhaseWatcherActivated = false;

    void ActivatePhaseWatcherOnce();  // GA_BossPhase1을 한 번 켜서 HP 80% 대기 시작
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
