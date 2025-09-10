// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "GA_Boss_Grab.generated.h"

class UAnimMontage;
class UGameplayEffect;
class UAbilityTask_PlayMontageAndWait;

UCLASS()
class MONSTERAIPLUGIN_API UGA_Boss_Grab : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UGA_Boss_Grab();

    // === ������ ���� ===
    // ���/������ ���� ��Ÿ�� (�����ʿ� ���� �ּ�)
    UPROPERTY(EditDefaultsOnly, Category = "Montage")
    TObjectPtr<UAnimMontage> BossGrabMontage = nullptr;

    // ��Ÿ�ָ� �� ���Ǻ��� ����(�̼��� �� Default)
    UPROPERTY(EditDefaultsOnly, Category = "Montage")
    FName StartSectionName = TEXT("Grab");

    // ANS�� BB���� Victim�� �д� ��츦 ����� ���
    UPROPERTY(EditDefaultsOnly, Category = "Blackboard")
    FName BB_TargetActorKey = TEXT("TargetActor");

    // Motion Warping ��� ����/Ÿ���/������ ����
    UPROPERTY(EditDefaultsOnly, Category = "Warp")
    bool bUseMotionWarping = true;

    UPROPERTY(EditDefaultsOnly, Category = "Warp")
    FName WarpTargetName = TEXT("Victim");

    UPROPERTY(EditDefaultsOnly, Category = "Warp")
    FName VictimSocketName = TEXT("spine_02");

    // (����) ���� �� ������ ������ GE
    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    TSubclassOf<UGameplayEffect> GE_GrabDamageClass;

    // (����) ������ ����(��Į��)
    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    float DamageLevel = 1.f;

    // �ٰ�����
    UPROPERTY(EditDefaultsOnly, Category = "Approach")
    bool bApproachBeforeGrab = true;

    UPROPERTY(EditDefaultsOnly, Category = "Approach", meta = (EditCondition = "bApproachBeforeGrab"))
    float ApproachAcceptanceRadius = 200.f;   // �� �Ÿ� �̳��� �ٷ� ���

    UPROPERTY(EditDefaultsOnly, Category = "Approach", meta = (EditCondition = "bApproachBeforeGrab"))
    float MaxApproachTime = 2.0f;             // �ִ� ���� �ð�(��)

    UPROPERTY(EditDefaultsOnly, Category = "Lock")
    FGameplayTag GrabBusyTag; // ��: "State.Boss.Grab.Active"

    UPROPERTY(EditDefaultsOnly, Category = "Blackboard")
    FName BB_CinematicLockKey = TEXT("bCinematicLock"); // BT���� ���� Ű

    //Cool Down
    UPROPERTY(EditDefaultsOnly, Category = "Grab|Cooldown")
    TSubclassOf<UGameplayEffect> GE_GrabCooldown;   // Duration�� GE (��: 4s)

    UPROPERTY(EditDefaultsOnly, Category = "Grab|Cooldown")
    FGameplayTag Tag_GrabCooldown; // "Cooldown.Boss.Grab" ���� �±�

protected:
    // ���� �� ĳ��
    TWeakObjectPtr<class ACharacter> CachedBoss;
    TWeakObjectPtr<class ACharacter> CachedVictim;

    UPROPERTY()
    TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

    // === UGameplayAbility ===
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility, bool bWasCancelled) override;

    UFUNCTION() void OnMontageCompleted();
    UFUNCTION() void OnMontageCancelled();

    FDelegateHandle MoveFinishedHandle;
    FAIRequestID MoveReqId;
    FTimerHandle ApproachTimeoutHandle;

    // ���� ����
    void StartGrabMontage();  // ���� ���� �� ȣ��
    void BeginApproach(class AAIController* AI, class ACharacter* Boss, class AActor* Target);
    void OnMoveFinished(FAIRequestID RequestID, const FPathFollowingResult& Result);
    void CleanupApproachBindings();
};