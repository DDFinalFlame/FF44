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
#include "Perception/AISense_Hearing.h"

// Debugging
#include "Kismet/KismetSystemLibrary.h"

// Class
#include "Data/PlayerTags.h"
#include "BasePlayerAttributeSet.h"
#include "BasePlayerController.h"
#include "BasePlayerState.h"
#include "Weapon/BaseWeapon.h"

float ABasePlayer::GetAttackPower_Implementation() const
{
	// 1) 가장 신뢰되는 경로: ASC에 등록된 AttributeSet에서 읽기
	const UBasePlayerAttributeSet* FromASC = nullptr;
	if (AbilitySystem)
	{
		FromASC = AbilitySystem->GetSet<UBasePlayerAttributeSet>();
		if (FromASC)
		{
			const float AP = FromASC->GetAttackPower();
			return FMath::IsFinite(AP) ? AP : 0.f;
		}
	}

	// 2) 폴백: 멤버로 보관 중인 AttributeSet에서 읽기
	if (auto Attribute = AbilitySystem->GetSet<UBasePlayerAttributeSet>())
	{
		const float AP = Attribute->GetAttackPower();
		return FMath::IsFinite(AP) ? AP : 0.f;
	}

	// 3) 최종 폴백
	return 0.f;
}

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
	GetCharacterMovement()->MaxAcceleration = 1000.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.f;

	OnCharacterMovementUpdated.AddDynamic(this, &ABasePlayer::CharacterMovementUpdated);

	//CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	//CameraBoom->SetupAttachment(RootComponent);
	//CameraBoom->AddLocalTransform(FTransform(FRotator(0.f, 0.f, 0.f), FVector(0.f, 80.f, 80.f)));
	//CameraBoom->TargetArmLength = 200.f;
	//CameraBoom->bUsePawnControlRotation = true;
	//CameraBoom->bDoCollisionTest = false; // 카메라 충돌 테스트 비활성화
	//// 카메라가 늦게 따라오는 설정
	////CameraBoom->bEnableCameraLag = true;
	////CameraBoom->bEnableCameraRotationLag = true;

	//FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	//FollowCamera->SetupAttachment(CameraBoom);
	//FollowCamera->bUsePawnControlRotation = false;		
}

void ABasePlayer::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (const ABasePlayerState* PS = GetPlayerState<ABasePlayerState>())
		if (auto ASC = PS->GetAbilitySystemComponent())
		{
			AbilitySystem = ASC;
			AbilitySystem->InitAbilityActorInfo(const_cast<ABasePlayerState*>(PS), this);
		}
}

// 리스닝 전용
void ABasePlayer::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (const ABasePlayerState* PS = GetPlayerState<ABasePlayerState>())
		if (auto ASC = PS->GetAbilitySystemComponent())
		{
			AbilitySystem = ASC;
			AbilitySystem->InitAbilityActorInfo(const_cast<ABasePlayerState*>(PS), this);
		}
}

void ABasePlayer::BeginPlay()
{
	Super::BeginPlay();

	MetaDataSetup();

	InitializeAbilities();
	InitializeEffects();
	InitializeGameplayTags();



	// Weapon를 월드에 생성 후 바로 장착
	Weapon = GetWorld()->SpawnActor<ABaseWeapon>(WeaponClass);
	if (Weapon)
	{
		Weapon->SetOwner(this);
		EquipWeapon();
	}
	else 
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon not spawned. Check WeaponClass."));
		return;
	}

	// Player Controller Set
	if (ABasePlayerController* PlayerController = Cast<ABasePlayerController>(GetController()))
	{
		FRotator ControlRotation = PlayerController->GetControlRotation();
		ControlRotation.Pitch = -10.f;

		if (AbilitySystem)
			PlayerController->InitUI(AbilitySystem);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Faild to cast Controller to APlayerController"));
		return;
	}

	// Delegate Bind
	OnPlayerMoveChanged.AddDynamic(this, &ABasePlayer::UpdateMoveType);
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

}

void ABasePlayer::InitializeAbilities()
{
	if (!AbilitySystem) return;

	// 나중에 첫 시작에서만 불러오도록 바꾸기
	// Level 옮길 시에 중복되어 들어갈 수 있음.
	AbilitySystem->GiveAbility(FGameplayAbilitySpec(EquipWeaponAbility));
	AbilitySystem->GiveAbility(FGameplayAbilitySpec(UnEquipWeaponAbility));
	AbilitySystem->GiveAbility(FGameplayAbilitySpec(HitAbility));
	AbilitySystem->GiveAbility(FGameplayAbilitySpec(DodgeAbility));
	AbilitySystem->GiveAbility(FGameplayAbilitySpec(DeathAbility));

	for (int32 i = 0; i < ComboAttackAbility.Num(); ++i)
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(ComboAttackAbility[i], 1, i));
}

void ABasePlayer::InitializeEffects()
{
	if (!AbilitySystem) return;

	EffectContext = AbilitySystem->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	if (StaminaRegenEffect)
	{
		FGameplayEffectSpecHandle Spec = AbilitySystem->MakeOutgoingSpec(StaminaRegenEffect, 1, EffectContext);
		if (Spec.IsValid())
		{
			Spec.Data->SetSetByCallerMagnitude(
				PlayerTags::Stat_Player_Stamina_RegenRate,
				AbilitySystem->GetSet<UBasePlayerAttributeSet>()->GetRegenRateStamina());

			AbilitySystem->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
	}

	if(StaminaRunEffect)
	{
		FGameplayEffectSpecHandle Spec = AbilitySystem->MakeOutgoingSpec(StaminaRunEffect, 1, EffectContext);
		if (Spec.IsValid())
		{
			AbilitySystem->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
	}
}

void ABasePlayer::InitializeGameplayTags()
{
	if (!AbilitySystem) return;

	// 초기 Ability Tag 설정
	AbilitySystem->AddLooseGameplayTag(PlayerTags::State_Player_Weapon_Equip);	
}

void ABasePlayer::MetaDataSetup()
{
	// Definition Load
	if (!PlayerDefinition.IsValid())
		PlayerDefinition.LoadSynchronous();

	UPlayerDefinition* def = PlayerDefinition.Get();

	if (!def)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerDefinition not set."));
		return;
	}

	if (AbilitySystem&&AttributeSetClass)
	{
		auto AttributeSet = NewObject<UAttributeSet>(this, AttributeSetClass);
		AttributeSet->InitFromMetaDataTable(def->PlayerMetaDataTable);

		AbilitySystem->AddAttributeSetSubobject(AttributeSet);
	}

}

void ABasePlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if(UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Movement Actions
		if(MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABasePlayer::Move);
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ABasePlayer::StopMove);
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

		if ((GetMovementComponent()->Velocity.Length() > 0.01f) && !IsMontagePlaying())
			SetDoInputMoving(true);
		else
			SetDoInputMoving(false);
	}
}

void ABasePlayer::StopMove()
{
	SetDoInputMoving(false);
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
	if (AbilitySystem->HasMatchingGameplayTag(UnEquipWeaponTag))
		GetCharacterMovement()->MaxWalkSpeed = UnEquipRunSpeed;
	else if (AbilitySystem->HasMatchingGameplayTag(EquipWeaponTag))
		GetCharacterMovement()->MaxWalkSpeed = EquipRunSpeed;
	else
		GetCharacterMovement()->MaxWalkSpeed = 0.f;

	if (GetVelocity().Length() > 0.f)
		SetEnableSprinting(true);
}

void ABasePlayer::StopRun(const FInputActionValue& Value)
{
	if (AbilitySystem->HasMatchingGameplayTag(UnEquipWeaponTag))
		GetCharacterMovement()->MaxWalkSpeed = UnEquipWalkSpeed;
	else if (AbilitySystem->HasMatchingGameplayTag(EquipWeaponTag))
		GetCharacterMovement()->MaxWalkSpeed = EquipWalkSpeed;
	else
		GetCharacterMovement()->MaxWalkSpeed = 0.f;

	SetEnableSprinting(false);
}

void ABasePlayer::Dodge(const FInputActionValue& Value)
{
	AbilitySystem->TryActivateAbilityByClass(DodgeAbility);
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
	for (int32 i = 0; i < ComboAttackAbility.Num(); ++i)
		AbilitySystem->TryActivateAbilityByClass(ComboAttackAbility[i]);
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

void ABasePlayer::CharacterMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity)
{
	auto OldSpeed = OldVelocity.Length();
	auto CurrentSpeed = GetVelocity().Length();

	if (CurrentSpeed <= 0.f)
	{
		CurrentNoiseLevel = 0.f;
		return;
	}

	if (FMath::IsNearlyEqual(OldSpeed, CurrentSpeed)) return;

	if(AbilitySystem->HasMatchingGameplayTag(UnEquipWeaponTag))
	{
		CurrentNoiseLevel = CurrentSpeed / (UnEquipRunSpeed + 200.f);
	}
	else if (AbilitySystem->HasMatchingGameplayTag(EquipWeaponTag))
	{
		CurrentNoiseLevel = CurrentSpeed / (EquipRunSpeed);
	}

	if (IsMontagePlaying())
	{
		CurrentNoiseLevel = 0.f;

		if (AbilitySystem->HasMatchingGameplayTag(PlayerTags::State_Player_Attack))
			CurrentNoiseLevel = 1.0f;
		else if (AbilitySystem->HasMatchingGameplayTag(PlayerTags::State_Player_Dodge))
			CurrentNoiseLevel = 0.8f;
	}

	UAISense_Hearing::ReportNoiseEvent(
		GetWorld(),
		GetActorLocation(),   // NoiseLocation
		CurrentNoiseLevel,    // Loudness(0~1 권장)
		this,                 // Instigator(보통 자기 자신)
		1000.f,               // MaxRange(0이면 무제한, 센서 범위로 제한)
		FName("Footstep")     // Tag
	);
}

void ABasePlayer::UpdateMoveType(bool _OldMoving, bool _OldSprinting)
{
	if (_OldMoving == DoInputMoving && _OldSprinting == EnableSprinting) return;

	if (DoInputMoving)
	{
		if (EnableSprinting)
		{
			AbilitySystem->AddLooseGameplayTag(PlayerTags::State_Player_Move_Run);
			AbilitySystem->RemoveLooseGameplayTag(PlayerTags::State_Player_Move_Walk);
		}
		else
		{
			AbilitySystem->AddLooseGameplayTag(PlayerTags::State_Player_Move_Walk);
			AbilitySystem->RemoveLooseGameplayTag(PlayerTags::State_Player_Move_Run);
		}
	}
	else
	{
		AbilitySystem->RemoveLooseGameplayTag(PlayerTags::State_Player_Move_Walk);
		AbilitySystem->RemoveLooseGameplayTag(PlayerTags::State_Player_Move_Run);
	}
}

bool ABasePlayer::IsMontagePlaying() const
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && AnimInstance->IsAnyMontagePlaying())
		return true;

	return false;
}

void ABasePlayer::SetDoInputMoving(bool _NewValue)
{
	auto OldValue = DoInputMoving;
	DoInputMoving = _NewValue; 
	OnPlayerMoveChanged.Broadcast(OldValue, EnableSprinting);
}

void ABasePlayer::SetEnableSprinting(bool _NewValue)
{
	auto OldValue = EnableSprinting;
	EnableSprinting = _NewValue;
	OnPlayerMoveChanged.Broadcast(DoInputMoving, OldValue);
}
void ABasePlayer::PlayerDead()
{
	IsDead = true;

	GetCharacterMovement()->DisableMovement();
	bUseControllerRotationYaw = false;
}

void ABasePlayer::PlayerAlive()
{
	IsDead = false;

	GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Walking;
	bUseControllerRotationYaw = true;
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