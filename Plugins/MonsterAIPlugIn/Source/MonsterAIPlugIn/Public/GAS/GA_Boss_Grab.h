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

    // === 에디터 세팅 ===
    // 잡기/던지기 전용 몽타주 (오프너와 별도 애셋)
    UPROPERTY(EditDefaultsOnly, Category = "Montage")
    TObjectPtr<UAnimMontage> BossGrabMontage = nullptr;

    // 몽타주를 이 섹션부터 시작(미설정 시 Default)
    UPROPERTY(EditDefaultsOnly, Category = "Montage")
    FName StartSectionName = TEXT("Grab");

    // ANS가 BB에서 Victim을 읽는 경우를 대비해 기록
    UPROPERTY(EditDefaultsOnly, Category = "Blackboard")
    FName BB_TargetActorKey = TEXT("TargetActor");

    // Motion Warping 사용 여부/타깃명/피해자 소켓
    UPROPERTY(EditDefaultsOnly, Category = "Warp")
    bool bUseMotionWarping = true;

    UPROPERTY(EditDefaultsOnly, Category = "Warp")
    FName WarpTargetName = TEXT("Victim");

    UPROPERTY(EditDefaultsOnly, Category = "Warp")
    FName VictimSocketName = TEXT("spine_02");

    // (선택) 성공 시 적용할 데미지 GE
    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    TSubclassOf<UGameplayEffect> GE_GrabDamageClass;

    // (선택) 데미지 레벨(스칼라)
    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    float DamageLevel = 1.f;

    // 다가가기
    UPROPERTY(EditDefaultsOnly, Category = "Approach")
    bool bApproachBeforeGrab = true;

    UPROPERTY(EditDefaultsOnly, Category = "Approach", meta = (EditCondition = "bApproachBeforeGrab"))
    float ApproachAcceptanceRadius = 200.f;   // 이 거리 이내면 바로 잡기

    UPROPERTY(EditDefaultsOnly, Category = "Approach", meta = (EditCondition = "bApproachBeforeGrab"))
    float MaxApproachTime = 2.0f;             // 최대 추적 시간(초)

    UPROPERTY(EditDefaultsOnly, Category = "Lock")
    FGameplayTag GrabBusyTag; // 예: "State.Boss.Grab.Active"

    UPROPERTY(EditDefaultsOnly, Category = "Blackboard")
    FName BB_CinematicLockKey = TEXT("bCinematicLock"); // BT에서 막을 키

    //Cool Down
    UPROPERTY(EditDefaultsOnly, Category = "Grab|Cooldown")
    TSubclassOf<UGameplayEffect> GE_GrabCooldown;   // Duration형 GE (예: 4s)

    UPROPERTY(EditDefaultsOnly, Category = "Grab|Cooldown")
    FGameplayTag Tag_GrabCooldown; // "Cooldown.Boss.Grab" 같은 태그

protected:
    // 실행 중 캐시
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

    // 내부 헬퍼
    void StartGrabMontage();  // 접근 끝난 뒤 호출
    void BeginApproach(class AAIController* AI, class ACharacter* Boss, class AActor* Target);
    void OnMoveFinished(FAIRequestID RequestID, const FPathFollowingResult& Result);
    void CleanupApproachBindings();
};