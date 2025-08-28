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
    // ========= ����/�Ķ���� =========
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Invuln")
    TSubclassOf<UGameplayEffect> GE_BossInvuln;                // ���� GE (Cue ����)

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Montage")
    UAnimMontage* StartMontage = nullptr;                      // ���� ���� ��Ÿ��

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Audio")
    USoundBase* StartSound = nullptr;                          // ���� ����

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Cast")
    UAnimMontage* CastLoopMontage = nullptr;                   // �ֱ��� ĳ���� ��Ÿ��

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Cast", meta = (ClampMin = "0.05"))
    float CastInterval = 2.5f;                                 // ĳ���� �ֱ�

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Summon")
    TSubclassOf<AActor> MinionClass;                           // ��ȯ BP

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Summon", meta = (ClampMin = "0"))
    int32 SpawnCountMin = 4;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Summon", meta = (ClampMin = "0"))
    int32 SpawnCountMax = 8;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Summon", meta = (ClampMin = "0.0"))
    float SpawnRadius = 900.f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Trigger", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StartHpRatioThreshold = 0.80f;                       // 80%

    // ========= ��Ÿ�� =========
    FActiveGameplayEffectHandle InvulnHandle;
    int32  AliveMinionCount = 0;
    bool   bPhaseStarted = false;

    FTimerHandle CastTimerHandle;
    FDelegateHandle HPChangeHandle;                            // HP ��ȭ ���� �ڵ�

    // ========= ���� �Լ� =========
    void StartPhase();                                         // ����/����/��ȯ/���� ����
    int32 SpawnMinions(AActor* Boss);
    void StartCastTick();                                      // ĳ���� ���� ����
    void DoCastOnce();                                         // 1ȸ ĳ����

    void BindHPThresholdWatch();
    void UnbindHPThresholdWatch();
    void OnHPChangedNative(const FOnAttributeChangeData& Data);

    UFUNCTION()
    void OnMinionDiedEvent(struct FGameplayEventData Payload); // Event.Minion.Died ����

    // ========= GA �������̵� =========
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility, bool bWasCancelled) override;
};