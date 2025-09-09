// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Monster/MonsterCharacter.h"
#include "GameplayEffectTypes.h"
#include "BossCharacter.generated.h"


UENUM(BlueprintType)
enum class EBossState_BB : uint8
{
    Idle,
    CombatReady,
    Phase0_Attack,
    InPhase1,
    Phase1_Attack,
    InPhase2,
    Phase2_Attack,
    InPhase3,
    Hit,
    Dead
};


class AMonsterBaseWeapon;
class ABossMeleeWeapon;
class UBossPhaseComponent;
class UBossArenaComponent;
class UBossCutsceneComponent;
class UBossPartsComponent;
class UBossRewardComponent;
class UAbilitySystemComponent;
class UGameplayEffect;
struct FOnAttributeChangeData;

UCLASS()
class MONSTERAIPLUGIN_API ABossCharacter : public AMonsterCharacter
{
    GENERATED_BODY()
public:
    ABossCharacter();
    virtual void BeginPlay() override; 

protected:

    // 무기 스폰/장착 유틸
    void SpawnAndAttachWeapons();

    // 여러 소켓 사용
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    TArray<FName> WeaponAttachSocketNames = { FName(TEXT("Muzzle_01")), FName(TEXT("Muzzle_02")) };


protected:

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
