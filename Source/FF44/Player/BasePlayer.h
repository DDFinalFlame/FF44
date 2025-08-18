#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

#include "Data/PlayerStatRow.h"
#include "Data/PlayerDefinition.h"

#include "BasePlayer.generated.h"

class AActor;
class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UGameplayAbility;

class UBasePlayerAttributeSet;

struct FInputActionValue;

UCLASS()
class FF44_API ABasePlayer : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABasePlayer();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	virtual void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
									  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
									  bool bFromSweep, const FHitResult& SweepResult);

///////////////////////////////////////////////////////////////////////////////////////////
///										Components										///
///////////////////////////////////////////////////////////////////////////////////////////
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAbilitySystemComponent* AbilitySystem;

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystem; }

///////////////////////////////////////////////////////////////////////////////////////
///										Data										///
///////////////////////////////////////////////////////////////////////////////////////
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attribute")
	UBasePlayerAttributeSet* AttributeSet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	TSubclassOf<UBasePlayerAttributeSet> AttributeSetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TSoftObjectPtr<UPlayerDefinition> PlayerDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	UDataTable* PlayerStatTable = nullptr;

	void ApplyInitStats(const FPlayerStatRow& Row, TSubclassOf<class UGameplayEffect> InitGE);

///////////////////////////////////////////////////////////////////////////////////////////
///										Abilities										///
///////////////////////////////////////////////////////////////////////////////////////////
protected:
	// Abilities
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TSubclassOf<UGameplayAbility> EquipWeaponAbility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TSubclassOf<UGameplayAbility> UnEquipWeaponAbility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TSubclassOf<UGameplayAbility> ComboAttackAbility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TSubclassOf<UGameplayAbility> HitAbility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TSubclassOf<UGameplayAbility> DodgeAbility;


///////////////////////////////////////////////////////////////////////////////////////////
///										Weapons											///
///////////////////////////////////////////////////////////////////////////////////////////
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	AActor* Weapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<class ABaseWeapon> WeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FName EquipSocket= "Equip_Weapon";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FName UnEquipSocket = "UnEquip_Weapon";

public:
	virtual void AttachWeapon(FName _Socket);
	virtual void DetachWeapon(FName _Socket);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void EquipWeapon();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void UnEquipWeapon();

public:
	AActor* GetWeapon() const { return Weapon; }
	void SetWeapon(AActor* _Weapon) { Weapon = _Weapon; }


///////////////////////////////////////////////////////////////////////////////////////////
///										Input											///
///////////////////////////////////////////////////////////////////////////////////////////

protected:
	// Movement Actions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InputAction")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InputAction")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InputAction")
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InputAction")
	UInputAction* DodgeAction;

	// Interact Action
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InputAction")
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InputAction")
	UInputAction* LockOnAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InputAction")
	UInputAction* ToggleCombatAction;

	// Combat Action
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InputAction")
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InputAction")
	UInputAction* SpecialAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InputAction")
	UInputAction* SkillAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Input")
	int CurrentInputDirection = 0; // 0: None, 1: Forward, 2: Backward, 3: Left, 4: Right

public:
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual int GetCurrentInputDirection() const { return CurrentInputDirection; }


protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Movement Actions
	virtual void Move(const FInputActionValue& Value);
	virtual void Look(const FInputActionValue& Value);
	virtual void Run(const FInputActionValue& Value);
	virtual void StopRun(const FInputActionValue& Value);
	virtual void Dodge(const FInputActionValue& Value);

	// Interact Actions
	virtual void Interact(const FInputActionValue& Value);
	virtual void LockOn(const FInputActionValue& Value);
	virtual void ToggleCombat(const FInputActionValue& Value);

	// Combat Actions
	virtual void Attack(const FInputActionValue& Value);
	virtual void SpecialAct(const FInputActionValue& Value);
	virtual void Skill(const FInputActionValue& Value);
};
