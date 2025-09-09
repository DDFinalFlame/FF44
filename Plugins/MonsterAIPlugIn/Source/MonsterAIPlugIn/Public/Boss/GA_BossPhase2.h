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

    UPROPERTY(EditDefaultsOnly, Category = "Phase2|End")
    float EndHpRatioThreshold = 0.20f;

    // ===== ����/���� =====
    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Invuln")
    TSubclassOf<UGameplayEffect> GE_BossInvuln;


    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Montage")
    UAnimMontage* StartMontage = nullptr;       // ������ ���� ����

    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Start")
    USoundBase* StartSound = nullptr;


    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Montage")
    UAnimMontage* EndMontage = nullptr;       // ������ ���� ����

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
    float ShockwaveRadius = 2100.f;

    // ===== ���� =====
    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Platform")
    TSubclassOf<AActor> PlatformClass; // �̹� ���ٴ� ���ǡ��� �ʿ� ��� �ִٸ� ��� X

    UPROPERTY(EditDefaultsOnly, Category = "Phase2|Platform")
    FGameplayTag Tag_PlatformTouched;  // ��: Event.Boss.PlatformTouched

    // 2������ ����� ����
    UPROPERTY(EditDefaultsOnly, Category = "Phase2|WeakPoint")
    TSubclassOf<AActor> WeakPointClass;

    UPROPERTY(EditDefaultsOnly, Category = "Phase2|WeakPoint")
    int32 WeakPointSpawnCount = 3;

    UPROPERTY(EditDefaultsOnly, Category = "Phase2|WeakPoint")
    float WeakPointSpawnRadius = 1000.f;

    UPROPERTY(EditDefaultsOnly, Category = "Phase2|WeakPoint")
    float WeakPointDamageToBoss = -10.f; // �ϳ� �μ� �� �������� �� ����
    // ���� HP�� ������ ���� �� ����� GE (SetByCaller: Data.Damage ��� ����)
    UPROPERTY(EditDefaultsOnly, Category = "Phase2|WeakPoint")
    TSubclassOf<UGameplayEffect> GE_WeakPointDamageToBoss;
    UPROPERTY(EditDefaultsOnly, Category = "Phase2|ShockWave")
    TSubclassOf<UGameplayEffect> GE_ShockWave;


    // ���� ����
    bool bPhaseStarted = false;
    FActiveGameplayEffectHandle InvulnHandle;
    FDelegateHandle HPChangeHandle;
    int32 TotalPlatforms = 0;
    int32 TouchedPlatforms = 0;


    FTimerHandle SmashTimerHandle;
    FTimerHandle LandFailSafeTimer;

    bool bStartingPhase2 = false;
    bool bSmashInProgress = false;
    bool bShouldEndAfterCurrentSmash = false; // �Ӱ� ���� �� ���� ���Ž� ���� ���
    bool bEnding = false;                     // ���� ������ �ߺ� ����
    bool bWaitingForLand = false;             // ���� ���


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
    UFUNCTION() void BeginStartSequence();
    UFUNCTION() void PlayStartMontageThenStartSmash();     // Start ��Ÿ�� ��� �� ���� ����
    UFUNCTION() void StartSmashLoop();                     // SmashTick Ÿ�̸� ����

    // ���� ����(�װ� �̹� ������ ��)
    UFUNCTION() void SmashTick();
    UFUNCTION() void OnLandEvent(FGameplayEventData Payload);
    UFUNCTION() void OnSmashMontageFinished();

    //// ���� ������
    UFUNCTION() void BeginEndSequence();
    UFUNCTION() void PlayEndMontageAndFinish();


    //// ����/�̺�Ʈ �ڵ鷯
    void SpawnWeakPoints();
    UFUNCTION() void OnWeakPointDestroyedEvent(FGameplayEventData Payload);

    // ���� ���� �Լ�
    void ApplyInvuln();
    void RemoveInvuln();

    UFUNCTION()
    void OnEndMontageFinished();  // ���� ��Ÿ�� ���� �� ȣ��

    // === Phase �� ���� ===
    void StartPhase();

};