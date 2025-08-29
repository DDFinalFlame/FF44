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
    // ===== ���� ���� =====
    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Trigger", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StartHpRatioThreshold = 0.40f;

    // ===== ����/���� =====
    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Invuln")
    TSubclassOf<UGameplayEffect> GE_Phase2Invuln;


    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Montage")
    UAnimMontage* PhaseStartMontage = nullptr;       // ������ ���� ����

    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Montage")
    UAnimMontage* PhaseEndMontage = nullptr;       // ������ ���� ����

    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Montage")
    UAnimMontage* GroundSmashMontage = nullptr;  // ���� ��Ÿ��


    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Tags")
    FGameplayTag LandEventTag = MonsterTags::Event_Boss_Land;

    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Smash", meta = (ClampMin = "0.1"))
    float SmashInterval = 2.5f;                  // ���� �ֱ�

    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Smash")
    float JumpZ = 900.f;                          // ���� ����(��Ʈ����̸� 0����)

    // GA_BossPhase2.h
    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Failsafe", meta = (ClampMin = "0.1"))
    float LandFailSafeSeconds = 1.0f;   // ���� �̺�Ʈ �ִ� ��� �ð�(��)


    // ===== ����� =====
    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Shockwave")
    TSubclassOf<AActor> ShockwaveActorClass;   // ���� ������ ����(Overlap or Ÿ�Ӷ���)

    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Shockwave")
    float ShockwaveDamage = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Shockwave")
    float ShockwaveRadius = 800.f;

    // ===== ���� =====
    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Platform")
    TSubclassOf<AActor> PlatformClass; // �̹� ���ٴ� ���ǡ��� �ʿ� ��� �ִٸ� ��� X

    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Platform")
    FGameplayTag Tag_PlatformTouched;  // ��: Event.Boss.PlatformTouched

    // ���� ����
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

    // === HP ��ó ===
    void BindHPThresholdWatch();
    void UnbindHPThresholdWatch();
    void OnHPChangedNative(const FOnAttributeChangeData& Data);


    //// ����/�� ������
    //UFUNCTION() void BeginStartSequence();
    //UFUNCTION() void PlayStartMontageThenStartLoop();
    //UFUNCTION() void StartSmashLoop();   // Ÿ�̸Ӹ� ����

    // ���� ����(�װ� �̹� ������ ��)
    UFUNCTION() void SmashTick();
    UFUNCTION() void OnLandEvent(FGameplayEventData Payload);
    UFUNCTION() void OnSmashMontageFinished();

    //// ���� ������
    //UFUNCTION() void BeginEndSequence();
    //UFUNCTION() void PlayEndMontageAndFinish();
    //UFUNCTION() void ForceLandIfStuck();

    //UFUNCTION() void OnEndMontageFinished();

    // === Phase �� ���� ===
    void StartPhase();
    //void DoJumpUp();         // ���� ����(��Ÿ��/Launch)
    //void WaitForSmash();     // ���� ���� ���
    //void OnSmashed();        // ����� ���� & ������
    //void BindPlatformTouch(); // ���� ��ġ �̺�Ʈ ����
    //void OnPlatformTouched(const FGameplayEventData& Payload);
    //bool AreAllPlatformsTouched() const;

    // ��ƿ
    //void ApplyInvuln(bool bEnable);
};