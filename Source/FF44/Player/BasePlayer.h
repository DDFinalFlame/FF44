#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

#include "Data/PlayerDefinition.h"
#include "AttackStatProvider.h"
#include "BasePlayer.generated.h"

class AActor;
class ABaseWeapon;
class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UGameplayAbility;

class UBasePlayerAttributeSet;
class UBasePlayerHUDWidget;

struct FInputActionValue;

UCLASS()
class FF44_API ABasePlayer : public ACharacter, public IAbilitySystemInterface, public IAttackStatProvider
{
	GENERATED_BODY()

public:
	// IAttackStatProvider Interface
	virtual float GetAttackPower_Implementation() const override;

public:
	ABasePlayer();

protected:
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	virtual void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
									  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
									  bool bFromSweep, const FHitResult& SweepResult);


///////////////////////////////////////////////////////////////////////////////////////////
///										Abilities										///
///////////////////////////////////////////////////////////////////////////////////////////
protected:
	// ASC
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystemComponent")
	UAbilitySystemComponent* AbilitySystem;

	// AbttributeSet
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	TSubclassOf<UBasePlayerAttributeSet> AttributeSetClass;

	// Abilities
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TSubclassOf<UGameplayAbility> EquipWeaponAbility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TSubclassOf<UGameplayAbility> UnEquipWeaponAbility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> ComboAttackAbility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TSubclassOf<UGameplayAbility> HitAbility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TSubclassOf<UGameplayAbility> DodgeAbility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TSubclassOf<UGameplayAbility> DeathAbility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AbilityTag")
	FGameplayTag EquipWeaponTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AbilityTag")
	FGameplayTag UnEquipWeaponTag;

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystem; }


///////////////////////////////////////////////////////////////////////////////////////
///										Cameras										///
///////////////////////////////////////////////////////////////////////////////////////
//protected:
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cameras")
//	UCameraComponent* FollowCamera;
//
//	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cameras")
//	USpringArmComponent* CameraBoom;


///////////////////////////////////////////////////////////////////////////////////////
///										Data										///
///////////////////////////////////////////////////////////////////////////////////////
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TSoftObjectPtr<UPlayerDefinition> PlayerDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	UDataTable* PlayerMetaDataTable = nullptr;


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


///////////////////////////////////////////////////////////////////////////////////////////
///										State											///
///////////////////////////////////////////////////////////////////////////////////////////
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool IsDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool IsInputMove = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	float CurrentNoiseLevel = 0.f;

private:
	UFUNCTION()
	void CharacterMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity);

	bool IsMontagePlaying() const;

public:
	UFUNCTION(BlueprintCallable, Category = "State")
	void PlayerDead();

	UFUNCTION(BlueprintCallable, Category = "State")
	void PlayerAlive();


///////////////////////////////////////////////////////////////////////////////////////////
///										Value											///
///////////////////////////////////////////////////////////////////////////////////////////
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value")
	float UnEquipWalkSpeed = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value")
	float UnEquipRunSpeed = 650.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value")
	float EquipWalkSpeed = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value")
	float EquipRunSpeed = 600.f;


///////////////////////////////////////////////////////////////////////////////////////////
///										Weapons											///
///////////////////////////////////////////////////////////////////////////////////////////

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	ABaseWeapon* Weapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<ABaseWeapon> WeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FName EquipSocket = "Equip_Weapon";

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
	ABaseWeapon* GetWeapon() const { return Weapon; }
	void SetWeapon(ABaseWeapon* _Weapon) { Weapon = _Weapon; }
};
