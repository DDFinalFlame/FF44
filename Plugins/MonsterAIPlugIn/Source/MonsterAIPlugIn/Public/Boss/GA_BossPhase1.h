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
class AMonsterCharacter;

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

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Montage")
    UAnimMontage* EndMontage = nullptr;


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
    bool bEnding = false;

    FTimerHandle CastTimerHandle;
    FDelegateHandle HPChangeHandle;                            // HP 변화 감시 핸들

    // ========= 내부 함수 =========
    void StartPhase();                                         // 무적/연출/소환/루프 시작
    int32 SpawnMinions(AActor* Boss);
    UFUNCTION()    void BeginStartSequence();
    UFUNCTION()    void PlayStartMontageThenStartCast();    // 시작몽타쥬
    UFUNCTION()    void StartCastTick();                    // 캐스팅 루프 시작
    UFUNCTION()    void BeginEndSequence();                 // 캐스트/몽타주 정리 시작
    UFUNCTION()    void PlayEndMontageAndFinish();          // End 몽타주 재생 → 끝나면 EndAbility
    void DoCastOnce();                                      // 1회 캐스팅

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


protected:
    // --- 미니언 초기화 재시도용 ---
    void EnqueueMinionInit(AMonsterCharacter* MC);
    void ProcessPendingMinionInits();
    bool TrySetupMinionBlackboard(AMonsterCharacter* MC, AActor* Player);
    AActor* GetPhaseTargetPlayer() const;

    /** 스폰 대기열 (BB/AIController 준비 안 됐을 때 재시도) */
    TArray<TWeakObjectPtr<AMonsterCharacter>> PendingInitMinions;

    /** 대기열 처리 타이머 */
    FTimerHandle MinionInitTimerHandle;

    /** 공격/전투 상태로 세팅할 값(실프로젝트 Enum에 맞게 수정) */
    uint8 DesiredMinionState = (uint8)3;


    UPROPERTY(EditDefaultsOnly, Category = "Phase|FallingRock")
    TSubclassOf<AActor> FallingRockClass;            // 바위 액터(BP/CPP)

    UPROPERTY(EditDefaultsOnly, Category = "Phase|FallingRock")
    int32 RocksPerCastMin = 3;

    UPROPERTY(EditDefaultsOnly, Category = "Phase|FallingRock")
    int32 RocksPerCastMax = 6;

    UPROPERTY(EditDefaultsOnly, Category = "Phase|FallingRock")
    float SpawnHeight = 1200.f;                      // 지면 위 스폰 높이

    UPROPERTY(EditDefaultsOnly, Category = "Phase|FallingRock")
    float PlayerAreaRadiusMin = 300.f;

    UPROPERTY(EditDefaultsOnly, Category = "Phase|FallingRock")
    float PlayerAreaRadiusMax = 700.f;


    // 시각용: 바위 자동 제거
    UPROPERTY(EditDefaultsOnly, Category = "Phase|FallingRock")
    float RockLifeSeconds = 6.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Phase1|Rock")
    float RockDamage = 20.f;


};