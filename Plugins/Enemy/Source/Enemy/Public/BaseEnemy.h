#pragma once  

#include "CoreMinimal.h"  
#include "GameFramework/Character.h"  
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "EnemyDefine.h"
#include "Engine/TargetPoint.h"
#include "Interfaces/EnemyWeaponControl.h"
#include "BaseEnemy.generated.h"

class AEnemyBaseWeapon;
struct FMonsterStatRow;
class UMonsterDefinition;
class UEnemyAttributeSet;
class UBehaviorTree;
class UEnemyRotationComponent;
class UAbilitySystemComponent;
class UGameplayEffect;

UCLASS()  
class ENEMY_API ABaseEnemy : public ACharacter  , public IAbilitySystemInterface, public IEnemyWeaponControl
{  
	GENERATED_BODY()  

// Base Info Section
protected:
	UPROPERTY(EditAnywhere, Category = "Monster | Data")
	EEnemyType EnemyType;

	/* 몬스터 초기화 데이터 ( MonsterAIPlugin 참조 ) **/
	UPROPERTY(EditAnywhere, Category = "Monster | Data")
	TSoftObjectPtr<UMonsterDefinition> MonsterDefinition;

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
protected:
	UPROPERTY(EditAnywhere, Category = "AI | Patrol")
	TArray<ATargetPoint*> PatrolPoints;

	UPROPERTY(VisibleAnywhere, Category = "AI | Patrol")
	int32 PatrolIndex = 0;

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

public:  
	ABaseEnemy();  

protected:  
	virtual void BeginPlay() override;  
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
public:	  
	virtual void Tick(float DeltaTime) override;  

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;  

public:
	FORCEINLINE ATargetPoint* GetPatrolPoint()
	{
		return PatrolPoints.Num() >= (PatrolIndex + 1) ? PatrolPoints[PatrolIndex] : nullptr;
	}
	FORCEINLINE void IncrementPatrolIndex()
	{
		PatrolIndex = (PatrolIndex + 1) % PatrolPoints.Num();
	}

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
};
