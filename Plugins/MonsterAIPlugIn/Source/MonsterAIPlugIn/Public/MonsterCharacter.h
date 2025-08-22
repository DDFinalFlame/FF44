// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MonsterStatRow.h"          
#include "MonsterDefinition.h"   
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AttackStatProvider.h"
#include "MonsterAttributeSet.h"
#include "MonsterCharacter.generated.h"


class UAbilitySystemComponent;
class UMonsterAttributeSet;
class USphereComponent;
class UGameplayEffect;
class AMonsterBaseWeapon;

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
class MONSTERAIPLUGIN_API AMonsterCharacter : public ACharacter, 
	public IAbilitySystemInterface, public IAttackStatProvider
{
	GENERATED_BODY()

public:
	AMonsterCharacter();
	virtual void Tick(float DeltaTime) override;

	// GAS 시스템용 함수들 : 몬스터의 ASC 제공
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** 서버 소유권 획득 시 ASC 재초기화 */
	virtual void PossessedBy(AController* _NewController) override;

	/** 클라 재소유(레플리케이션) 시 ASC 재초기화 */
	virtual void OnRep_PlayerState() override;

	virtual void Attack();

	//상태 변경 함수
	UFUNCTION(BlueprintCallable, Category = "State")
	void SetMonsterState(EMonsterState NewState);

	UFUNCTION(BlueprintPure, Category = "State")
	EMonsterState GetMonsterState() const { return CurrentState; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	EMonsterState StartState = EMonsterState::Patrol; 

	// 기습 종료 후 일반 루프로 전환
	UFUNCTION(BlueprintCallable, Category = "AI")
	void FinishAmbush();  // AmbushReady -> CombatReady 로 전환

protected:

	virtual void BeginPlay() override;

	
	// =========================
    //  1) 데이터/스탯/정의 로딩
    // =========================

    /** 몬스터 정의(Definition에 따라서 사용) */
	UPROPERTY(EditAnywhere, Category = "Monster|Data")
	TSoftObjectPtr<UMonsterDefinition> MonsterDefinition;

	/** 스탯 데이터 테이블 */
	UPROPERTY(EditDefaultsOnly, Category = "Monster|Data")
	UDataTable* MonsterStatTable = nullptr;

	// SetByCaller 값 주입 헬퍼
	void ApplyInitStats(const FMonsterStatRow& Row, TSubclassOf<class UGameplayEffect> InitGE);


	// 기습 속도 제어(기본값/배율)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Ambush")
	float DefaultWalkSpeed = 0.f;          // DT에서 받은 기본 이동속도 캐시

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Ambush")
	float AmbushSpeedRate = 3.0f;          

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Ambush")
	float AttackSpeedRate = 2.0f;          

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
	float DetectDistanceCache = 0.f; 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|State")
	float FallbackDetectDistance = 800.f;


	// --- [공격 공통 훅: 파생 전용 가상 함수] ---
protected:
	// 파생: 공격용 히트박스 생성 및 부착(머리/칼 등).
	virtual void SetupAttackCollision() {}

	// 파생: 공격 타이밍에 히트박스 On/Off(AnimNotify에서 호출)
	virtual void ActivateAttackHitbox(bool bEnable);

	// 파생: 데미지 수치(또는 DT/Def 기반 계산)
	virtual float GetAttackDamage() const { return 10.f; }

	// 파생 후처리 훅 (이펙트/사운드 등).
	virtual void OnAttackHit(AActor* _Victim) {}

	// --- [공격 공통 로직: 베이스에서 제공] ---
protected:
	// 등록된 모든 공격 히트박스(파생이 RegisterHitbox로 등록)
	UPROPERTY()
	TArray<UPrimitiveComponent*> AttackHitboxes;

	// 한 번의 스윙에서 중복 히트 방지
	TSet<TWeakObjectPtr<AActor>> HitActorsThisSwing;

	// 현재 스윙 활성화 여부(Overlap을 받을지 여부)
	bool bAttackActive = false;

	// 파생 히트박스 생성 시 반드시 호출해서 베이스에 등록(Overlap 바인딩)
	void RegisterHitbox(UPrimitiveComponent* Comp);

	// 공용: 스윙 시작/종료(AnimNotify_Begin/End에서 호출하면 편합니다)
public:
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void BeginAttackWindow(); // bAttackActive=true, HitActorsThisSwing 초기화

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EndAttackWindow();   // bAttackActive=false

protected:
	// 공통 Overlap 핸들러(등록된 모든 AttackHitboxes가 여기로 바인딩)
	UFUNCTION()
	void OnAttackHitboxBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// 맞은 대상에게 실제 처리(GAS 이벤트/데미지 등)
	void ApplyMeleeHitTo(AActor* Victim, const FHitResult& Hit);

protected:
		// BP에서 무기 클래스를 지정
		UPROPERTY(EditAnywhere, Category = "Weapon")
		TSubclassOf<AMonsterBaseWeapon> WeaponClass;

		// 런타임에 스폰된 무기 인스턴스
		UPROPERTY()
		AMonsterBaseWeapon* Weapon = nullptr;

public:
		// 필요하면 접근자
		FORCEINLINE AMonsterBaseWeapon* GetWeapon() const { return Weapon; }
		void PushAttackCollision();
		void PopAttackCollision();

private:
		int32 AttackCollisionDepth = 0;
		UPROPERTY(EditDefaultsOnly, Category = "Collision")
		FName DefaultProfile = TEXT("Monster_Default");
		UPROPERTY(EditDefaultsOnly, Category = "Collision")
		FName AttackingProfile = TEXT("Monster_Attacking");

		void ApplyCollisionProfile();


public:
	virtual float GetAttackPower_Implementation() const override;
};
