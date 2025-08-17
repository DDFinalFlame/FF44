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

	// GAS �ý��ۿ� �Լ���
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	virtual void Attack();

	//���� ���� �Լ�
	UFUNCTION(BlueprintCallable, Category = "State")
	void SetMonsterState(EMonsterState NewState);

	UFUNCTION(BlueprintPure, Category = "State")
	EMonsterState GetMonsterState() const { return CurrentState; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	EMonsterState StartState = EMonsterState::Patrol;   // �⺻�� �Ϲ� ����

	// (�ɼ�) BT���� ȣ���ϱ� ���� ����: ��� ���� �� �Ϲ� ������ ��ȯ
	UFUNCTION(BlueprintCallable, Category = "AI")
	void FinishAmbush();  // AmbushReady -> CombatReady �� ��ȯ

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Monster|Data")
	TSoftObjectPtr<UMonsterDefinition> MonsterDefinition;

	UPROPERTY(EditDefaultsOnly, Category = "Monster|Data")
	UDataTable* MonsterStatTable = nullptr;   // DT_MonsterStats ����

	// SetByCaller �� ���� ����
	void ApplyInitStats(const FMonsterStatRow& Row, TSubclassOf<class UGameplayEffect> InitGE);


	// ��� �ӵ� ����(�⺻��/����)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Ambush")
	float DefaultWalkSpeed = 0.f;          // DT���� ���� �⺻ �̵��ӵ� ĳ��

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Ambush")
	float AmbushSpeedRate = 3.0f;          // DT ���� �� ����� �⺻ ����(��: 1.4��)

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
	float DetectDistanceCache = 0.f; // DT���� ä��

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|State")
	float FallbackDetectDistance = 800.f; // ���� �⺻��
};
