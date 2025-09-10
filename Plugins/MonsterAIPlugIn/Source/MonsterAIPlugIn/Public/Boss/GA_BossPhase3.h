// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayEffectTypes.h"          
#include "AbilitySystemComponent.h"       
#include "TimerManager.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Sound/SoundBase.h"
#include "GA_BossPhase3.generated.h"

class UAnimMontage;
class UGameplayEffect;
class AActor;
class ACharacter;

UCLASS()
class MONSTERAIPLUGIN_API UGA_BossPhase3 : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_BossPhase3();

	/** ===== Overrides ===== */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;

	UPROPERTY(EditAnywhere, Category = "P3|AI")
	FName BB_TargetActorKeyName = TEXT("TargetActor");

	UPROPERTY(EditAnywhere, Category = "P3|AI")
	float ApproachAcceptanceRadius = 150.f;


protected:
	/** ====== Timed ticks ====== */
	UFUNCTION() void Tick_Rock();
	UFUNCTION() void Tick_Attack();
	UFUNCTION() void Tick_Minion();

	/** ====== HP watch ====== */
	void BindHP();
	void UnbindHP();
	void OnHPChanged(const FOnAttributeChangeData& Data);

	/** ====== Invulnerability ====== */
	void ApplyInvuln();
	void RemoveInvuln();

	/** Phase2에서 쓰던 콜백 재사용 (AddDynamic 사용하므로 UFUNCTION 필요) */
	UFUNCTION() void OnWeakPointDestroyedEvent(FGameplayEventData Payload);
	UFUNCTION() void OnSmashMontageFinished();   // 몽타주 인터럽트/취소/블렌드아웃 공용
	UFUNCTION() void OnMinionDied(FGameplayEventData Payload);
protected:
	/** ===================== Config ===================== */

	// --- Rock drop ---
	UPROPERTY(EditDefaultsOnly, Category = "P3|Rock")
	TSubclassOf<AActor> FallingRockClass;

	UPROPERTY(EditDefaultsOnly, Category = "P3|Rock", meta = (ClampMin = "0.05"))
	float RockInterval = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "P3|Rock")
	float RockRadiusMin = 600.f;

	UPROPERTY(EditDefaultsOnly, Category = "P3|Rock")
	float RockRadiusMax = 1200.f;

	// --- Attacks ---
	UPROPERTY(EditDefaultsOnly, Category = "P3|Attack")
	TArray<UAnimMontage*> AttackMontages;           // 최대 3개 예상

	UPROPERTY(EditDefaultsOnly, Category = "P3|Attack", meta = (ClampMin = "0.1"))
	float AttackIntervalMin = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "P3|Attack", meta = (ClampMin = "0.1"))
	float AttackIntervalMax = 3.0f;

	// --- Minions ---
	UPROPERTY(EditDefaultsOnly, Category = "P3|Minion")
	TSubclassOf<ACharacter> MinionClass;

	UPROPERTY(EditDefaultsOnly, Category = "P3|Minion", meta = (ClampMin = "0.1"))
	float MinionInterval = 8.f;

	UPROPERTY(EditDefaultsOnly, Category = "P3|Minion", meta = (ClampMin = "0"))
	int32 MinionSpawnCountEachTime = 2;

	// --- Invuln GE (P3 전용) ---
	UPROPERTY(EditDefaultsOnly, Category = "P3|Invuln")
	TSubclassOf<UGameplayEffect> GE_BossInvuln_P3;

	// --- Weakpoint / Damage (Phase2 재사용용) ---
	UPROPERTY(EditDefaultsOnly, Category = "P3|WeakPoint")
	TSubclassOf<AActor> WeakPointClass;

	UPROPERTY(EditDefaultsOnly, Category = "P3|WeakPoint")
	float WeakPointDamageToBoss = -10.f;

	UPROPERTY(EditDefaultsOnly, Category = "P3|WeakPoint")
	TSubclassOf<UGameplayEffect> GE_WeakPointDamageToBoss;

	/** ===================== Runtime ===================== */
	// Timers
	FTimerHandle RockTimer;
	FTimerHandle AttackTimer;
	FTimerHandle MinionTimer;

	// HP delegate
	FDelegateHandle HPChangeHandle;

	// Current state
	UPROPERTY(Transient)
	bool bAttackPlaying = false;

	// Invuln GE handle
	FActiveGameplayEffectHandle InvulnHandle;

	//보스 주입
	UPROPERTY(BlueprintReadOnly, Category = "Owner")
	TWeakObjectPtr<AActor> OwnerBoss;

	UFUNCTION(BlueprintCallable)
	void SetOwnerBoss(AActor* InBoss) { OwnerBoss = InBoss; }

	UFUNCTION(BlueprintCallable)
	AActor* GetOwnerBoss() const { return OwnerBoss.Get(); }

	// 남은 소환수 제거
		// 3페이즈 동안 스폰된 미니언 추적용
	UPROPERTY() TArray<TWeakObjectPtr<ACharacter>> PhaseMinions;

	// 보스 사망 시: 남아있는 미니언 체력을 0으로 깎기
	UFUNCTION() void ForceKillPhaseMinions();

	// 보스가 죽으면 소환몹 데미지
	UPROPERTY(EditDefaultsOnly, Category = "Boss|P3")
	TSubclassOf<UGameplayEffect> GE_ForceKillMinion;
	// 보스 사망플래그
	UPROPERTY() bool bDisableWeakpointSpawns = false;

	// 보스 죽었을때,
	void EnterDeathCleanupFromP3(bool bTriggerDeathGA = true);

protected:
	// Sound 관련 Phase1과 동일한 설정값
	UPROPERTY(EditAnywhere, Category = "Phase3|Rock")
	USoundBase* RockImpactSound = nullptr;

	UPROPERTY(EditAnywhere, Category = "Phase3|Rock", meta = (ClampMin = "0.0"))
	float RockImpactSfxDelay = 0.25f; // 소환 후 몇 초 뒤에 재생할지

	// 타이머에서 호출할 함수(람다 X)
	UFUNCTION()
	void PlayImpactSfxAt(FVector ImpactLoc);

	// 필요 시 타이머 핸들 모아둘 수 있음(취소 용도)
	TArray<FTimerHandle> RockSfxTimerHandles;
private:
	FAIRequestID CurrentMoveId;
	FDelegateHandle MoveFinishedHandle;
	bool bMovingToAttack = false;

private:
	AActor* GetTargetFromBlackboard() const;
	void StartMoveToTargetOrAttack();
	void OnMoveFinished(FAIRequestID RequestID, const FPathFollowingResult& Result);
	void StartRandomAttackMontageOrReschedule();
};
