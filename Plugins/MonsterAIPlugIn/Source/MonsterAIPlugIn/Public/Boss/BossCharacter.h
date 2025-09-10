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
class UBlackboardComponent;

UCLASS()
class MONSTERAIPLUGIN_API ABossCharacter : public AMonsterCharacter
{
    GENERATED_BODY()
public:
    ABossCharacter();
    virtual void BeginPlay() override; 
    virtual void Tick(float _dt) override;
protected:

    // 무기 스폰/장착 유틸
    void SpawnAndAttachWeapons();

    // 여러 소켓 사용
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    TArray<FName> WeaponAttachSocketNames = { FName(TEXT("Muzzle_01")), FName(TEXT("Muzzle_02")) };


protected:

    // == BGM ==
    UPROPERTY(EditAnywhere, Category = "Cinematic|BGM")
    USoundBase* BattleBGM = nullptr;

    UPROPERTY(EditAnywhere, Category = "Cinematic|BGM", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BattleBGMVolume = 0.7f;

    UPROPERTY(Transient)
    UAudioComponent* BattleBGMComp = nullptr;

    UPROPERTY(EditAnywhere, Category = "Cinematic|BGM")
    float BattleBGMFadeOut = 0.8f;

    UPROPERTY(Transient)
    UBlackboardComponent* CachedBB = nullptr;

    void StartBattleBGM();
    void StopBattleBGM(bool bFadeOut = true, float FadeOutTime = 0.8f);
    void TryCacheBlackboardOnce();
    //
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

protected:
    UPROPERTY(EditAnywhere, Category = "Audio")
    USoundBase* DeathSound = nullptr;   // 죽을 때 재생할 사운드

    bool bDeathSoundPlayed = false;     // 중복 방지

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class UCameraComponent* Grab;
	
    // 죽을 때 스폰할 액터
    UPROPERTY(EditAnywhere, Category = "Death|Spawn")
    TSubclassOf<AActor> DeathSpawnClass1;

    UPROPERTY(EditAnywhere, Category = "Death|Spawn")
    TSubclassOf<AActor> DeathSpawnClass2;

    // 위치 오프셋 (보스 기준)
    UPROPERTY(EditAnywhere, Category = "Death|Spawn")
    float DeathSpawnBackOffset = 150.f;   // 뒤로
    UPROPERTY(EditAnywhere, Category = "Death|Spawn")
    float DeathSpawnSideOffset = 120.f;   // 좌/우
    UPROPERTY(EditAnywhere, Category = "Death|Spawn")
    float DeathSpawnZOffset = 0.f;        // 높이 보정

    bool bDeathExtrasSpawned = false;

    UFUNCTION()
    void SpawnDeathSideActors();
};
