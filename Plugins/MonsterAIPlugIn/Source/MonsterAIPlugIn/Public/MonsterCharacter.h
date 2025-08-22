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

	// GAS �ý��ۿ� �Լ��� : ������ ASC ����
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** ���� ������ ȹ�� �� ASC ���ʱ�ȭ */
	virtual void PossessedBy(AController* _NewController) override;

	/** Ŭ�� �����(���ø����̼�) �� ASC ���ʱ�ȭ */
	virtual void OnRep_PlayerState() override;

	virtual void Attack();

	//���� ���� �Լ�
	UFUNCTION(BlueprintCallable, Category = "State")
	void SetMonsterState(EMonsterState NewState);

	UFUNCTION(BlueprintPure, Category = "State")
	EMonsterState GetMonsterState() const { return CurrentState; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	EMonsterState StartState = EMonsterState::Patrol; 

	// ��� ���� �� �Ϲ� ������ ��ȯ
	UFUNCTION(BlueprintCallable, Category = "AI")
	void FinishAmbush();  // AmbushReady -> CombatReady �� ��ȯ

protected:

	virtual void BeginPlay() override;

	
	// =========================
    //  1) ������/����/���� �ε�
    // =========================

    /** ���� ����(Definition�� ���� ���) */
	UPROPERTY(EditAnywhere, Category = "Monster|Data")
	TSoftObjectPtr<UMonsterDefinition> MonsterDefinition;

	/** ���� ������ ���̺� */
	UPROPERTY(EditDefaultsOnly, Category = "Monster|Data")
	UDataTable* MonsterStatTable = nullptr;

	// SetByCaller �� ���� ����
	void ApplyInitStats(const FMonsterStatRow& Row, TSubclassOf<class UGameplayEffect> InitGE);


	// ��� �ӵ� ����(�⺻��/����)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Ambush")
	float DefaultWalkSpeed = 0.f;          // DT���� ���� �⺻ �̵��ӵ� ĳ��

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Ambush")
	float AmbushSpeedRate = 3.0f;          

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Ambush")
	float AttackSpeedRate = 2.0f;          

	// ����
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

	//BB�� ���µ���ȭ
	void SyncStateToBlackboard();

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDebugLOS = true;


public:
	// --- ����� �ǰ� �׽�Ʈ�� ---

	UPROPERTY(EditAnywhere, Category = "HitTest|Trigger")
	TSubclassOf<class UGameplayEffect> TestDamageGE; // �����Ϳ��� GE_TestDamage ����

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HitTest|Trigger")
	USphereComponent* HitTestTrigger = nullptr;

	// ���� Ʈ���� ������ ��ٿ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitTest|Trigger")
	float HitCooldown = 0.5f;

	// ������ Ʈ���� �ð�
	double LastHitTime = -1000.0;

	// �÷��̾�� ������ ���� �� �ݹ�
	UFUNCTION()
	void OnHitTestBegin(UPrimitiveComponent* _OverlappedComp, AActor* _OtherActor,
		UPrimitiveComponent* _OtherComp, int32 _OtherBodyIndex,
		bool _bFromSweep, const FHitResult& _SweepResult);

	// ���� �ǰ� ó��(�̺�Ʈ ����+������ ����)
	void TriggerHitReact(AActor* _InstigatorActor);

	UFUNCTION(BlueprintPure, Category = "Monster|Data")
	UMonsterDefinition* GetMonsterDef() const { return MonsterDefinition.Get(); }

	// ���� ó��
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


	// --- [���� ���� ��: �Ļ� ���� ���� �Լ�] ---
protected:
	// �Ļ�: ���ݿ� ��Ʈ�ڽ� ���� �� ����(�Ӹ�/Į ��).
	virtual void SetupAttackCollision() {}

	// �Ļ�: ���� Ÿ�ֿ̹� ��Ʈ�ڽ� On/Off(AnimNotify���� ȣ��)
	virtual void ActivateAttackHitbox(bool bEnable);

	// �Ļ�: ������ ��ġ(�Ǵ� DT/Def ��� ���)
	virtual float GetAttackDamage() const { return 10.f; }

	// �Ļ� ��ó�� �� (����Ʈ/���� ��).
	virtual void OnAttackHit(AActor* _Victim) {}

	// --- [���� ���� ����: ���̽����� ����] ---
protected:
	// ��ϵ� ��� ���� ��Ʈ�ڽ�(�Ļ��� RegisterHitbox�� ���)
	UPROPERTY()
	TArray<UPrimitiveComponent*> AttackHitboxes;

	// �� ���� �������� �ߺ� ��Ʈ ����
	TSet<TWeakObjectPtr<AActor>> HitActorsThisSwing;

	// ���� ���� Ȱ��ȭ ����(Overlap�� ������ ����)
	bool bAttackActive = false;

	// �Ļ� ��Ʈ�ڽ� ���� �� �ݵ�� ȣ���ؼ� ���̽��� ���(Overlap ���ε�)
	void RegisterHitbox(UPrimitiveComponent* Comp);

	// ����: ���� ����/����(AnimNotify_Begin/End���� ȣ���ϸ� ���մϴ�)
public:
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void BeginAttackWindow(); // bAttackActive=true, HitActorsThisSwing �ʱ�ȭ

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EndAttackWindow();   // bAttackActive=false

protected:
	// ���� Overlap �ڵ鷯(��ϵ� ��� AttackHitboxes�� ����� ���ε�)
	UFUNCTION()
	void OnAttackHitboxBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// ���� ��󿡰� ���� ó��(GAS �̺�Ʈ/������ ��)
	void ApplyMeleeHitTo(AActor* Victim, const FHitResult& Hit);

protected:
		// BP���� ���� Ŭ������ ����
		UPROPERTY(EditAnywhere, Category = "Weapon")
		TSubclassOf<AMonsterBaseWeapon> WeaponClass;

		// ��Ÿ�ӿ� ������ ���� �ν��Ͻ�
		UPROPERTY()
		AMonsterBaseWeapon* Weapon = nullptr;

public:
		// �ʿ��ϸ� ������
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
