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

UENUM(BlueprintType)
enum class EMonsterState : uint8
{
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* AttackMontage;

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

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Monster|Data")
	TSoftObjectPtr<UMonsterDefinition> MonsterDefinition;

	UPROPERTY(EditDefaultsOnly, Category = "Monster|Data")
	UDataTable* MonsterStatTable = nullptr;   // DT_MonsterStats 지정

	// SetByCaller 값 주입 헬퍼
	void ApplyInitStats(const FMonsterStatRow& Row, TSubclassOf<class UGameplayEffect> InitGE);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EMonsterState CurrentState = EMonsterState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UMonsterAttributeSet> AttributeSet;
	

};
