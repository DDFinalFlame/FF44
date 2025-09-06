#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MonsterTags.h"
#include "GA_BossPhase2.generated.h"

UCLASS()
class MONSTERAIPLUGIN_API UGA_BossPhase2 : public UGameplayAbility
{
    GENERATED_BODY()
public:
    UGA_BossPhase2();

protected:
    // ===== 시작 조건 =====
    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Trigger", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StartHpRatioThreshold = 0.40f;

    UPROPERTY(EditDefaultsOnly, Category = "Phase2|End")
    float EndHpRatioThreshold = 0.20f;

    // ===== 연출/무적 =====
    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Invuln")
    TSubclassOf<UGameplayEffect> GE_BossInvuln;


    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Montage")
    UAnimMontage* StartMontage = nullptr;       // 페이즈 시작 연출

    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Start")
    USoundBase* StartSound = nullptr;


    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Montage")
    UAnimMontage* EndMontage = nullptr;       // 페이즈 종료 연출

    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Montage")
    UAnimMontage* GroundSmashMontage = nullptr;  // 점프 몽타쥬


    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Tags")
    FGameplayTag LandEventTag = MonsterTags::Event_Boss_Land;

    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Smash", meta = (ClampMin = "0.1"))
    float SmashInterval = 2.5f;                  // 쿵쿵 주기

    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Smash")
    float JumpZ = 900.f;                          // 점프 세기(루트모션이면 0으로)

    // GA_BossPhase2.h
    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Failsafe", meta = (ClampMin = "0.1"))
    float LandFailSafeSeconds = 1.0f;   // 착지 이벤트 최대 대기 시간(초)


    // ===== 충격파 =====
    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Shockwave")
    TSubclassOf<AActor> ShockwaveActorClass;   // 링이 퍼지는 액터(Overlap or 타임라인)

    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Shockwave")
    float ShockwaveDamage = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Shockwave")
    float ShockwaveRadius = 2100.f;

    // ===== 발판 =====
    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Platform")
    TSubclassOf<AActor> PlatformClass; // 이미 “바닥 발판”이 맵에 깔려 있다면 사용 X

    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Platform")
    FGameplayTag Tag_PlatformTouched;  // 예: Event.Boss.PlatformTouched

    // 2페이즈 생기는 엑터
    UPROPERTY(EditDefaultsOnly, Category = "Phase2|WeakPoint")
    TSubclassOf<AActor> WeakPointClass;

    UPROPERTY(EditDefaultsOnly, Category = "Phase2|WeakPoint")
    int32 WeakPointSpawnCount = 3;

    UPROPERTY(EditDefaultsOnly, Category = "Phase2|WeakPoint")
    float WeakPointSpawnRadius = 1000.f;

    UPROPERTY(EditDefaultsOnly, Category = "Phase2|WeakPoint")
    float WeakPointDamageToBoss = -10.f; // 하나 부술 때 보스에게 줄 피해
    // 보스 HP를 실제로 깎을 때 사용할 GE (SetByCaller: Data.Damage 사용 권장)
    UPROPERTY(EditDefaultsOnly, Category = "Phase2|WeakPoint")
    TSubclassOf<UGameplayEffect> GE_WeakPointDamageToBoss;
    UPROPERTY(EditDefaultsOnly, Category = "Phase2|ShockWave")
    TSubclassOf<UGameplayEffect> GE_ShockWave;


    // 내부 상태
    bool bPhaseStarted = false;
    FActiveGameplayEffectHandle InvulnHandle;
    FDelegateHandle HPChangeHandle;
    int32 TotalPlatforms = 0;
    int32 TouchedPlatforms = 0;


    FTimerHandle SmashTimerHandle;
    FTimerHandle LandFailSafeTimer;

    bool bStartingPhase2 = false;
    bool bSmashInProgress = false;
    bool bShouldEndAfterCurrentSmash = false; // 임계 도달 후 현재 스매시 종료 대기
    bool bEnding = false;                     // 엔딩 시퀀스 중복 방지
    bool bWaitingForLand = false;             // 착지 대기


    // === Ability overrides ===
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility, bool bWasCancelled) override;

    // === HP 워처 ===
    void BindHPThresholdWatch();
    void UnbindHPThresholdWatch();
    void OnHPChangedNative(const FOnAttributeChangeData& Data);


    //// 시작/끝 시퀀스
    UFUNCTION() void BeginStartSequence();
    UFUNCTION() void PlayStartMontageThenStartSmash();     // Start 몽타주 재생 후 루프 시작
    UFUNCTION() void StartSmashLoop();                     // SmashTick 타이머 시작

    // 루프 동작(네가 이미 구현한 것)
    UFUNCTION() void SmashTick();
    UFUNCTION() void OnLandEvent(FGameplayEventData Payload);
    UFUNCTION() void OnSmashMontageFinished();

    //// 종료 시퀀스
    UFUNCTION() void BeginEndSequence();
    UFUNCTION() void PlayEndMontageAndFinish();


    //// 스폰/이벤트 핸들러
    void SpawnWeakPoints();
    UFUNCTION() void OnWeakPointDestroyedEvent(FGameplayEventData Payload);

    // 무적 관련 함수
    void ApplyInvuln();
    void RemoveInvuln();

    UFUNCTION()
    void OnEndMontageFinished();  // 엔딩 몽타주 종료 시 호출

    // === Phase 본 로직 ===
    void StartPhase();

};