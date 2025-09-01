#pragma once  

#include "CoreMinimal.h"  
#include "GameFramework/Character.h"  
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "EnemyRotationComponent.h"

#include "EnemyStateConfig.h"
#include "BT/BTService_SelectBehavior.h"
#include "Interfaces/EnemyWeaponControl.h"
#include "BaseEnemy.generated.h"

class UHitReactionDataAsset;
class AEnemyBaseWeapon;
class UBehaviorTree;
class UEnemyRotationComponent;
class UAbilitySystemComponent;
class UGameplayEffect;

class UMonsterDefinition;
class UMonsterAttributeSet;
struct FMonsterStatRow;

UCLASS()  
class ENEMY_API ABaseEnemy : public ACharacter, public IAbilitySystemInterface, public IEnemyWeaponControl
{  
	GENERATED_BODY()  

// Base Info Section
protected:
	UPROPERTY(EditAnywhere, Category = "Monster | Data")
	EEnemyType EnemyType;

	/* 몬스터 초기화 데이터 ( MonsterAIPlugin 참조 ) **/
	UPROPERTY(EditAnywhere, Category = "Monster | Data")
	TSoftObjectPtr<UMonsterDefinition> MonsterDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster | Data")
	TSubclassOf<UMonsterAttributeSet> AttributeSetClass;

// GAS Section
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS | Ability")
	UAbilitySystemComponent* AbilitySystemComponent;
public:
	UPROPERTY(EditDefaultsOnly, Category = "GAS | Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	/* AttributeSet 초기값 Data Table **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS | Attribute")
	UDataTable* EnemyDataTable;

// AI Section
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Behavior")
	UBehaviorTree* BehaviorTree;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Behavior")
	FName BehaviorKeyName;

// State ( Behavior )
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | Behavior")
	FEnemyStateConfig BehaviorConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI | Behavior")
	EAIBehavior CurrentBehavior;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI | Behavior")
	bool IsCurrentBehaviorEnd = true;

// Component Section
protected:
	/* Rotation **/
	UPROPERTY(VisibleAnywhere)
	UEnemyRotationComponent* RotationComponent;

// Combat
protected:
	/* Weapon **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | Weapon CDO");
	TSubclassOf<AEnemyBaseWeapon> WeaponClass;

	UPROPERTY(BlueprintReadOnly, Category = "Combat | Weapon");
	AEnemyBaseWeapon* Weapon;

	/* Montage **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | Montage")
	UHitReactionDataAsset* EnemyMontageData;

// Death 연출
protected:
	FTimerHandle DissolveTimerHandle;
public:  
	ABaseEnemy();  

protected:  
	virtual void BeginPlay() override;  
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
public:	  
	virtual void Tick(float DeltaTime) override;  

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;  

// GAS Section
public:
	void GiveDefaultAbilities();
	bool RequestAbilityByTag(FGameplayTag AbilityTag);
	void InitializeAttributeSet();
	/* Set Attribute ( MonsterAIPlugin 참조 ) **/
	void ApplyInitStats(const FMonsterStatRow& Row, TSubclassOf<class UGameplayEffect> InitGE);

// Weapon Control
public:
	virtual void ActivateWeaponCollision() override;
	virtual void DeactivateWeaponCollision() override;
	virtual bool IsAttackSuccessful() override;

// AI - State
public:
	void SetState(EAIBehavior NewBehavior);
	bool ChangeState(EAIBehavior NewBehavior);
	bool CheckCurrentBehavior(EAIBehavior NewBehavior);
	bool IsCurrentStateInterruptible();
	void EndCurrentBehavior();
	FORCEINLINE EAIBehavior GetCurrentBehavior() const { return CurrentBehavior; }
	/* Death 애니메이션 처리 중 **/
	void OnDeath();
	/* Death 애니메이션 종료 **/
	void EndDeath();
	/* 월드에서 사라지기 **/
	void StartDissolve();

// Montage
public:
	UAnimMontage* GetHitMontage(EHitDirection Direction) const;
	UAnimMontage* GetDieMontage() const;
	UAnimMontage* GetAttackMontage(FGameplayTagContainer TargetTags) const;

// Rotation
public:
	FORCEINLINE void SetRotationTarget(const FVector& Location) const
	{
		if (RotationComponent)
		{
			RotationComponent->SetTargetLocation(Location);	
		}
	}
};
