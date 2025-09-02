#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

#include "Data/PlayerDefinition.h"
#include "Interface/AttackStatProvider.h"
#include "BasePlayer.generated.h"

class AActor;
class ABaseWeapon;
class UCameraComponent;
class USpringArmComponent;
class UArrowComponent;
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

	struct FGameplayEffectContextHandle EffectContext;

	// AbttributeSet
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	TSubclassOf<UBasePlayerAttributeSet> AttributeSetClass;

	class UBasePlayerAttributeSet* BaseAttribute;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TSubclassOf<UGameplayAbility> PotionAbility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AbilityTag")
	FGameplayTag EquipWeaponTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AbilityTag")
	FGameplayTag UnEquipWeaponTag;

	// Effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TSubclassOf<UGameplayEffect> StaminaRegenEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TSubclassOf<UGameplayEffect> StaminaRunEffect;


public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystem; }
	
protected:
	virtual void InitializeAbilities();
	virtual void InitializeEffects();
	virtual void InitializeGameplayTags();

///////////////////////////////////////////////////////////////////////////////////////
///										Cameras										///
///////////////////////////////////////////////////////////////////////////////////////
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class UBasePlayerCameraManager> BaseCameraManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cameras")
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cameras")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cameras")
	UArrowComponent* CameraUnequipLook;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cameras")
	UArrowComponent* CameraEquipLook;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cameras")
	UArrowComponent* CameraZoomInLook;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cameras")
	UArrowComponent* CameraRightMoveLook;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cameras")
	UArrowComponent* CameraLeftMoveLook;

public:
	void ZeroControllerPitch();
	class UBasePlayerCameraManager* GetCameraManager() { return BaseCameraManager; }


///////////////////////////////////////////////////////////////////////////////////////
///										Data										///
///////////////////////////////////////////////////////////////////////////////////////
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TSoftObjectPtr<UPlayerDefinition> PlayerDefinition;

private:
	void MetaDataSetup();


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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InputAction")
	UInputAction* ItemSlot_1Action;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Input")
	int CurrentInputDirection = 0; // 0: None, 1: Forward, 2: Backward, 3: Left, 4: Right

public:
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual int GetCurrentInputDirection() const { return CurrentInputDirection; }

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Movement Actions
	virtual void Move(const FInputActionValue& Value);
	virtual void StopMove();
	virtual void Look(const FInputActionValue& Value);
	virtual void Running(const FInputActionValue& Value);
	virtual void StopRun(const FInputActionValue& Value);
	virtual void Dodge(const FInputActionValue& Value);

	// Interact Actions
	virtual void Interact(const FInputActionValue& Value);
	virtual void LockOn(const FInputActionValue& Value);
	virtual void ToggleCombat(const FInputActionValue& Value);
	virtual void ItemSlot_1(const FInputActionValue& Value);

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
	bool DoInputMoving = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool EnableSprinting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	float CurrentNoiseLevel = 0.f;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerMoveChanged, bool, DoInputMoving, bool, EnableSprinting);
	UPROPERTY(BlueprintAssignable)
	FOnPlayerMoveChanged OnPlayerMoveChanged;

protected:
	UFUNCTION()
	void CharacterMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity);

	UFUNCTION()
	void UpdateMoveType(bool _Moving, bool _Sprinting);

	bool IsMontagePlaying() const;

	void SetDoInputMoving(bool _NewValue);
	void SetEnableSprinting(bool _NewValue);


public:
	UFUNCTION(BlueprintCallable, Category = "State")
	void PlayerDead();

	UFUNCTION(BlueprintCallable, Category = "State")
	void PlayerAlive();


///////////////////////////////////////////////////////////////////////////////////////////
///										Value											///
///////////////////////////////////////////////////////////////////////////////////////////



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
