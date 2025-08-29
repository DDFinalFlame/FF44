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

    // ===== 연출/무적 =====
    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Invuln")
    TSubclassOf<UGameplayEffect> GE_Phase2Invuln;


    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Montage")
    UAnimMontage* PhaseStartMontage = nullptr;       // 페이즈 시작 연출

    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Montage")
    UAnimMontage* PhaseEndMontage = nullptr;       // 페이즈 종료 연출

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
    float ShockwaveRadius = 800.f;

    // ===== 발판 =====
    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Platform")
    TSubclassOf<AActor> PlatformClass; // 이미 “바닥 발판”이 맵에 깔려 있다면 사용 X

    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Platform")
    FGameplayTag Tag_PlatformTouched;  // 예: Event.Boss.PlatformTouched

    // 내부 상태
    bool bPhaseStarted = false;
    FActiveGameplayEffectHandle InvulnHandle;
    FDelegateHandle HPChangeHandle;
    int32 TotalPlatforms = 0;
    int32 TouchedPlatforms = 0;


    FTimerHandle SmashTimerHandle;
    FTimerHandle LandFailSafeTimer;

    bool bSmashInProgress = false;
    bool bEnding = false;
    bool bWaitingForLand = false;


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
    //UFUNCTION() void BeginStartSequence();
    //UFUNCTION() void PlayStartMontageThenStartLoop();
    //UFUNCTION() void StartSmashLoop();   // 타이머만 세팅

    // 루프 동작(네가 이미 구현한 것)
    UFUNCTION() void SmashTick();
    UFUNCTION() void OnLandEvent(FGameplayEventData Payload);
    UFUNCTION() void OnSmashMontageFinished();

    //// 종료 시퀀스
    //UFUNCTION() void BeginEndSequence();
    //UFUNCTION() void PlayEndMontageAndFinish();
    //UFUNCTION() void ForceLandIfStuck();

    //UFUNCTION() void OnEndMontageFinished();

    // === Phase 본 로직 ===
    void StartPhase();
    //void DoJumpUp();         // 점프 시작(몽타주/Launch)
    //void WaitForSmash();     // 착지 시점 대기
    //void OnSmashed();        // 충격파 생성 & 데미지
    //void BindPlatformTouch(); // 발판 터치 이벤트 수신
    //void OnPlatformTouched(const FGameplayEventData& Payload);
    //bool AreAllPlatformsTouched() const;

    // 유틸
    //void ApplyInvuln(bool bEnable);
};