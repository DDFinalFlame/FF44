#include "Player/BasePlayer.h"

// Components
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

// Debugging
#include "Kismet/KismetSystemLibrary.h"

// Class
#include "Weapon/BaseWeapon.h"
#include "BasePlayerAttributeSet.h"

ABasePlayer::ABasePlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	// Components Setup
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ABasePlayer::OnCapsuleBeginOverlap);

	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxWalkSpeed = 100.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->AddLocalTransform(FTransform(FRotator(0.f, 0.f, 0.f), FVector(0.f, 80.f, 80.f)));
	CameraBoom->TargetArmLength = 200.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bDoCollisionTest = false; // 카메라 충돌 테스트 비활성화
	// 카메라가 늦게 따라오는 설정
	//CameraBoom->bEnableCameraLag = true;
	//CameraBoom->bEnableCameraRotationLag = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;

	AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
	if(AbilitySystem)
	{		
		AttributeSet = CreateDefaultSubobject<UBasePlayerAttributeSet>(TEXT("AttributeSet"));
	}
}

void ABasePlayer::BeginPlay()
{
	Super::BeginPlay();

	// Definition Load
	if (!PlayerDefinition.IsValid())
		PlayerDefinition.LoadSynchronous();

	UPlayerDefinition* def = PlayerDefinition.Get();

	if(!def)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerDefinition not set."));
		return;
	}

	// Data Load
	if (PlayerStatTable)
	{
		if(FPlayerStatRow* row = PlayerStatTable->FindRow<FPlayerStatRow>(def->StatRowName, TEXT("Player Stat Row")))
		{
			// 초기 Stats 적용
			ApplyInitStats(*row, def->InitStatGE_SetByCaller);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Player Stat Row not found in DataTable."));
			return;
		}
	}
	
	// Player Controller Set
	AController* PlayerController = GetController();
	if(!PlayerController)
	{
		FRotator ControlRotation = PlayerController->GetControlRotation();
		ControlRotation.Pitch = -10.f;
		return;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Faild to cast Controller to APlayerController"));
	}

	// Weapon를 월드에 생성 후 바로 장착
	Weapon = GetWorld()->SpawnActor<AActor>(WeaponClass);
	auto WeaponActor = Cast<ABaseWeapon>(Weapon);

	if (WeaponActor)
		WeaponActor->SetOwner(this);
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon is not a valid ABaseWeapon instance."));
		return;
	}

	EquipWeapon();

	// 초기 Ability Tag 설정
	AbilitySystem->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Player.Weapon.Equip")));

	// Ability 등록
	if (AbilitySystem)
	{
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(EquipWeaponAbility));
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(UnEquipWeaponAbility));
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(ComboAttackAbility));	
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(HitAbility));
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(DodgeAbility));

		if (AttributeSetClass)
			AttributeSet = NewObject<UBasePlayerAttributeSet>(this, AttributeSetClass);
	}	
}

void ABasePlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentInputDirection = 0;
}

void ABasePlayer::OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
										UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
										bool bFromSweep, const FHitResult& SweepResult)
{
	//if (OtherActor && OtherActor != this)
	//{
	//	auto Monster = Cast<AMonsterCharacter>(OtherActor);
	//	if (Monster)
	//	{
	//		AbilitySystem->TryActivateAbilityByClass(HitAbility);
	//	}
	//}
}

void ABasePlayer::ApplyInitStats(const FPlayerStatRow& Row, TSubclassOf<class UGameplayEffect> InitGE)
{
	if (!AbilitySystem || !InitGE) return;

	FGameplayEffectContextHandle Ech = AbilitySystem->MakeEffectContext();
	FGameplayEffectSpecHandle Esh = AbilitySystem->MakeOutgoingSpec(InitGE, 1.f, Ech);

	if (!Esh.IsValid()) return;

	const FGameplayTag Tag_Health = FGameplayTag::RequestGameplayTag(FName("Player.Stat.Health"));
	const FGameplayTag Tag_Stamina = FGameplayTag::RequestGameplayTag(FName("Player.Stat.Stamina"));
	const FGameplayTag Tag_Attack = FGameplayTag::RequestGameplayTag(FName("Player.Stat.AttackPower"));

	Esh.Data->SetSetByCallerMagnitude(Tag_Health, Row.MaxHealth);
	Esh.Data->SetSetByCallerMagnitude(Tag_Stamina, Row.MaxStamina);
	Esh.Data->SetSetByCallerMagnitude(Tag_Attack, Row.AttackPower);

	AbilitySystem->ApplyGameplayEffectSpecToSelf(*Esh.Data.Get());

	UKismetSystemLibrary::PrintString(this, 
		FString::Printf(TEXT("Health : %f, Stamina : %f, Attack : %f"), 
			Row.MaxHealth, 
			Row.MaxStamina,
			Row.AttackPower));
}

void ABasePlayer::AttachWeapon(FName _Socket)
{
	if (Weapon && GetMesh())
	{
		Weapon->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::KeepRelativeTransform,
			_Socket);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to attach weapon: Invalid Weapon or Mesh"));
	}
}

void ABasePlayer::DetachWeapon(FName _Socket)
{
	if (Weapon && GetMesh())
	{
		Weapon->DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to detach weapon: Invalid Weapon or Mesh"));
	}
}

void ABasePlayer::EquipWeapon()
{
	AttachWeapon(EquipSocket);

	// Equip Animation 재생
}

void ABasePlayer::UnEquipWeapon()
{
	// UnEquip Animation 재생

	AttachWeapon(UnEquipSocket);
}

void ABasePlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if(UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Movement Actions
		if(MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABasePlayer::Move);
		}
		if(LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABasePlayer::Look);
		}
		if(SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ABasePlayer::Run);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ABasePlayer::StopRun);
		}
		if(DodgeAction)
		{
			EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &ABasePlayer::Dodge);
		}

		// Interact Actions
		if(InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ABasePlayer::Interact);
		}
		if(LockOnAction)
		{
			EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Triggered, this, &ABasePlayer::LockOn);
		}
		if(ToggleCombatAction)
		{
			EnhancedInputComponent->BindAction(ToggleCombatAction, ETriggerEvent::Triggered, this, &ABasePlayer::ToggleCombat);
		}

		// Combat Actions
		if(AttackAction)
		{
			EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ABasePlayer::Attack);
		}
		if(SpecialAction)
		{
			EnhancedInputComponent->BindAction(SpecialAction, ETriggerEvent::Triggered, this, &ABasePlayer::SpecialAct);
		}
		if(SkillAction)
		{
			EnhancedInputComponent->BindAction(SkillAction, ETriggerEvent::Triggered, this, &ABasePlayer::Skill);
		}
	}
	else
	{
		Super::SetupPlayerInputComponent(PlayerInputComponent);
	}
}

void ABasePlayer::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller)
	{
		if(MovementVector.X > 0.f)
		{
			CurrentInputDirection = 4; // Right
		}
		else if(MovementVector.X < 0.f)
		{
			CurrentInputDirection = 3; // Left
		}
		else if(MovementVector.Y > 0.f)
		{
			CurrentInputDirection = 1; // Forward
		}
		else if(MovementVector.Y < 0.f)
		{
			CurrentInputDirection = 2; // Backward
		}

		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector FowardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(FowardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ABasePlayer::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller)
	{
		FRotator ControlRotation = Controller->GetControlRotation();

		float NewPitch = FMath::Clamp(ControlRotation.Pitch - LookAxisVector.Y, -40.f, 30.f);
		float NewYaw = ControlRotation.Yaw + LookAxisVector.X;

		Controller->SetControlRotation(FRotator(NewPitch, NewYaw, 0.f));
	}
}

void ABasePlayer::Run(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
}

void ABasePlayer::StopRun(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = 100.f;
}

void ABasePlayer::Dodge(const FInputActionValue& Value)
{
	AbilitySystem->TryActivateAbilityByClass(DodgeAbility);
	
	// PlayMontage
}

void ABasePlayer::Interact(const FInputActionValue& Value)
{
	// Change State

	// PlayMontage
}

void ABasePlayer::LockOn(const FInputActionValue& Value)
{
	// Change State

	// Camera Lock-On Logic
}

void ABasePlayer::ToggleCombat(const FInputActionValue& Value)
{
	// Change State

	// PlayMontage
	
	// Attach Socket
}

void ABasePlayer::Attack(const FInputActionValue& Value)
{
	AbilitySystem->TryActivateAbilityByClass(ComboAttackAbility);	
}

void ABasePlayer::SpecialAct(const FInputActionValue& Value)
{
	// Change State

	// PlayMontage
}

void ABasePlayer::Skill(const FInputActionValue& Value)
{
	// Change State

	// PlayMontage
}