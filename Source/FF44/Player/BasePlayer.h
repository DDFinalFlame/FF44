#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BasePlayer.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputAction;
struct FInputActionValue;

UCLASS()
class FF44_API ABasePlayer : public ACharacter
{
	GENERATED_BODY()

protected:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* CameraBoom;

protected:
	// Input Actions
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

public:
	ABasePlayer();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
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
