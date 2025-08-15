#pragma once  

#include "CoreMinimal.h"  
#include "GameFramework/Character.h"  
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Engine/TargetPoint.h"
#include "BaseEnemy.generated.h"  

class UEnemyRotationComponent;
class UAbilitySystemComponent;

UCLASS()  
class ENEMY_API ABaseEnemy : public ACharacter  , public IAbilitySystemInterface
{  
	GENERATED_BODY()  

// GAS Section
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;
public:
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> PerformAttackAbility;

// AI - Patrol
protected:
	UPROPERTY(EditAnywhere, Category = "AI | Patrol")
	TArray<ATargetPoint*> PatrolPoints;

	UPROPERTY(VisibleAnywhere, Category = "AI | Patrol")
	int32 PatrolIndex = 0;

// Component
protected:
	UPROPERTY(VisibleAnywhere)
	UEnemyRotationComponent* RotationComponent;
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

public:
	bool RequestAttack();
};
