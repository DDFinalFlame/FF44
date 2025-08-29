// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Monster/MonsterCharacter.h"
#include "BossCharacter.generated.h"


UENUM(BlueprintType)
enum class EBossState_BB : uint8
{
    Idle,
    Phase0_Attack,
    InPhase1,
    Phase1_Attack,
    InPhase2,
    Phase2_Attack,
    Hit,
    Dead
};



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

    UPROPERTY(EditDefaultsOnly, Category = "Boss|GA")
    TSubclassOf<UGameplayAbility> Phase2AbilityClass;

    bool bPhaseWatcherActivated = false;

    void ActivatePhaseWatcherOnce();  // GA_BossPhase1을 한 번 켜서 HP 80% 대기 시작

public:
    UFUNCTION(BlueprintCallable)
    void SetBossState_EBB(uint8 NewState); // BP Enum 값 그대로 넣을 수 있게 uint8

    UFUNCTION(BlueprintCallable)
    void SetBossState_Name(FName BBKeyName, uint8 NewState); // 필요 시 키이름 지정

    virtual void Landed(const FHitResult& Hit) override;

public:
    UFUNCTION(BlueprintCallable)
    void SetBlackboardTargetActor(FName BBKeyName, AActor* NewTarget);
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
