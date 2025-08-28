#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"              // FOnAttributeChangeData
#include "GA_BossPhase1.generated.h"

class UGameplayEffect;
class UAnimMontage;
class USoundBase;
class AActor;
class UMonsterAttributeSet;

UCLASS()
class MONSTERAIPLUGIN_API UGA_BossPhase1 : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UGA_BossPhase1();

protected:
    // ========= 에셋/파라미터 =========
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Invuln")
    TSubclassOf<UGameplayEffect> GE_BossInvuln;                // 무적 GE (Cue 포함)

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Montage")
    UAnimMontage* StartMontage = nullptr;                      // 시작 연출 몽타주

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Audio")
    USoundBase* StartSound = nullptr;                          // 시작 사운드

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Cast")
    UAnimMontage* CastLoopMontage = nullptr;                   // 주기적 캐스팅 몽타주

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Cast", meta = (ClampMin = "0.05"))
    float CastInterval = 2.5f;                                 // 캐스팅 주기

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Summon")
    TSubclassOf<AActor> MinionClass;                           // 소환 BP

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Summon", meta = (ClampMin = "0"))
    int32 SpawnCountMin = 4;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Summon", meta = (ClampMin = "0"))
    int32 SpawnCountMax = 8;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Summon", meta = (ClampMin = "0.0"))
    float SpawnRadius = 900.f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Trigger", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StartHpRatioThreshold = 0.80f;                       // 80%

    // ========= 런타임 =========
    FActiveGameplayEffectHandle InvulnHandle;
    int32  AliveMinionCount = 0;
    bool   bPhaseStarted = false;

    FTimerHandle CastTimerHandle;
    FDelegateHandle HPChangeHandle;                            // HP 변화 감시 핸들

    // ========= 내부 함수 =========
    void StartPhase();                                         // 무적/연출/소환/루프 시작
    int32 SpawnMinions(AActor* Boss);
    void StartCastTick();                                      // 캐스팅 루프 시작
    void DoCastOnce();                                         // 1회 캐스팅

    void BindHPThresholdWatch();
    void UnbindHPThresholdWatch();
    void OnHPChangedNative(const FOnAttributeChangeData& Data);

    UFUNCTION()
    void OnMinionDiedEvent(struct FGameplayEventData Payload); // Event.Minion.Died 수신

    // ========= GA 오버라이드 =========
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility, bool bWasCancelled) override;
};