// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MonsterStatRow.h"          
#include "MonsterDefinition.h"   
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "MonsterCharacter.generated.h"


class UAbilitySystemComponent;
class UMonsterAttributeSet;
class USphereComponent;
class UGameplayEffect;

UENUM(BlueprintType)
enum class EMonsterState : uint8
{
	AmbushReady,
	Idle,
	Patrol,
	CombatReady,
	Attack,
	Hit,
	Knockback,
	Dead
};


UCLASS()
class MONSTERAIPLUGIN_API AMonsterCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMonsterCharacter();
	virtual void Tick(float DeltaTime) override;

	// GAS 시스템용 함수들
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	virtual void Attack();

	//상태 변경 함수
	UFUNCTION(BlueprintCallable, Category = "State")
	void SetMonsterState(EMonsterState NewState);

	UFUNCTION(BlueprintPure, Category = "State")
	EMonsterState GetMonsterState() const { return CurrentState; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	EMonsterState StartState = EMonsterState::Patrol;   // 기본은 일반 순찰

	// (옵션) BT에서 호출하기 쉬운 헬퍼: 기습 종료 후 일반 루프로 전환
	UFUNCTION(BlueprintCallable, Category = "AI")
	void FinishAmbush();  // AmbushReady -> CombatReady 로 전환

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Monster|Data")
	TSoftObjectPtr<UMonsterDefinition> MonsterDefinition;

	UPROPERTY(EditDefaultsOnly, Category = "Monster|Data")
	UDataTable* MonsterStatTable = nullptr;   // DT_MonsterStats 지정

	// SetByCaller 값 주입 헬퍼
	void ApplyInitStats(const FMonsterStatRow& Row, TSubclassOf<class UGameplayEffect> InitGE);


	// 기습 속도 제어(기본값/배율)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Ambush")
	float DefaultWalkSpeed = 0.f;          // DT에서 받은 기본 이동속도 캐시

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Ambush")
	float AmbushSpeedRate = 3.0f;          // DT 없을 때 사용할 기본 배율(예: 1.4배)

	// 헬퍼
	UFUNCTION(BlueprintCallable, Category = "AI|Ambush")
	void BeginAmbushBoost();

	UFUNCTION(BlueprintCallable, Category = "AI|Ambush")
	void EndAmbushBoost();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EMonsterState CurrentState = EMonsterState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UMonsterAttributeSet> AttributeSet;

	//BB와 상태동기화
	void SyncStateToBlackboard();

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDebugLOS = true;


public:
	// --- 디버그 피격 테스트용 ---

	UPROPERTY(EditAnywhere, Category = "HitTest|Trigger")
	TSubclassOf<class UGameplayEffect> TestDamageGE; // 에디터에서 GE_TestDamage 지정

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HitTest|Trigger")
	USphereComponent* HitTestTrigger = nullptr;

	// 연속 트리거 방지용 쿨다운
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitTest|Trigger")
	float HitCooldown = 0.5f;

	// 마지막 트리거 시각
	double LastHitTime = -1000.0;

	// 플레이어와 오버랩 시작 시 콜백
	UFUNCTION()
	void OnHitTestBegin(UPrimitiveComponent* _OverlappedComp, AActor* _OtherActor,
		UPrimitiveComponent* _OtherComp, int32 _OtherBodyIndex,
		bool _bFromSweep, const FHitResult& _SweepResult);

	// 실제 피격 처리(이벤트 전송+데미지 적용)
	void TriggerHitReact(AActor* _InstigatorActor);

	UFUNCTION(BlueprintPure, Category = "Monster|Data")
	UMonsterDefinition* GetMonsterDef() const { return MonsterDefinition.Get(); }

	// 죽음 처리
	void OnDeadTagChanged(const FGameplayTag Tag, int32 NewCount);

	//protected:
	//	UPROPERTY(EditAnywhere, Category = "Monster|Data")
	//	TSoftObjectPtr<UMonsterDefinition> MonsterDefinition;

protected:
	void UpdateTransition_PatrolToCombatReady();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|State")
	float DetectDistanceCache = 0.f; // DT에서 채움

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|State")
	float FallbackDetectDistance = 800.f; // 안전 기본값
};
