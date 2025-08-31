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
    // ========= ����/�Ķ���� =========
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Invuln")
    TSubclassOf<UGameplayEffect> GE_BossInvuln;                // ���� GE (Cue ����)

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Montage")
    UAnimMontage* StartMontage = nullptr;                      // ���� ���� ��Ÿ��

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Montage")
    UAnimMontage* EndMontage = nullptr;


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
    bool bEnding = false;

    FTimerHandle CastTimerHandle;
    FDelegateHandle HPChangeHandle;                            // HP ��ȭ ���� �ڵ�

    // ========= ���� �Լ� =========
    void StartPhase();                                         // ����/����/��ȯ/���� ����
    int32 SpawnMinions(AActor* Boss);
    UFUNCTION()    void BeginStartSequence();
    UFUNCTION()    void PlayStartMontageThenStartCast();    // ���۸�Ÿ��
    UFUNCTION()    void StartCastTick();                    // ĳ���� ���� ����
    UFUNCTION()    void BeginEndSequence();                 // ĳ��Ʈ/��Ÿ�� ���� ����
    UFUNCTION()    void PlayEndMontageAndFinish();          // End ��Ÿ�� ��� �� ������ EndAbility
    void DoCastOnce();                                      // 1ȸ ĳ����

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


protected:
    // --- �̴Ͼ� �ʱ�ȭ ��õ��� ---
    void EnqueueMinionInit(AMonsterCharacter* MC);
    void ProcessPendingMinionInits();
    bool TrySetupMinionBlackboard(AMonsterCharacter* MC, AActor* Player);
    AActor* GetPhaseTargetPlayer() const;

    /** ���� ��⿭ (BB/AIController �غ� �� ���� �� ��õ�) */
    TArray<TWeakObjectPtr<AMonsterCharacter>> PendingInitMinions;

    /** ��⿭ ó�� Ÿ�̸� */
    FTimerHandle MinionInitTimerHandle;

    /** ����/���� ���·� ������ ��(��������Ʈ Enum�� �°� ����) */
    uint8 DesiredMinionState = (uint8)3;


    UPROPERTY(EditDefaultsOnly, Category = "Phase|FallingRock")
    TSubclassOf<AActor> FallingRockClass;            // ���� ����(BP/CPP)

    UPROPERTY(EditDefaultsOnly, Category = "Phase|FallingRock")
    int32 RocksPerCastMin = 3;

    UPROPERTY(EditDefaultsOnly, Category = "Phase|FallingRock")
    int32 RocksPerCastMax = 6;

    UPROPERTY(EditDefaultsOnly, Category = "Phase|FallingRock")
    float SpawnHeight = 1200.f;                      // ���� �� ���� ����

    UPROPERTY(EditDefaultsOnly, Category = "Phase|FallingRock")
    float PlayerAreaRadiusMin = 300.f;

    UPROPERTY(EditDefaultsOnly, Category = "Phase|FallingRock")
    float PlayerAreaRadiusMax = 700.f;


    // �ð���: ���� �ڵ� ����
    UPROPERTY(EditDefaultsOnly, Category = "Phase|FallingRock")
    float RockLifeSeconds = 6.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Phase1|Rock")
    float RockDamage = 20.f;


};