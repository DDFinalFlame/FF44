#pragma once

// Enemy State
UENUM(BlueprintType)
enum class EAIBehavior : uint8
{
	Idle,
	Patrol,
	MeleeAttack,
	RangeAttack,
	Approach,
	Investigate,
	Hit,
	Die
};

// Enemy Type
UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	Iron UMETA(DisplayName = "Iron"),
	Archer UMETA(DisplayName = "Archer"),
	Ghost UMETA(DisplayName = "Ghost"),
	Sevarog UMETA(DisplayName = "Sevarog")
};

// Get FName By Enum
FName GetEnemyRowName(EEnemyType CharacterType);


// Weapon Type
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	LeftHand,
	RightHand,
	Bow,
};


// Attack Type
UENUM(BlueprintType)
enum class EAttackType : uint8
{
	Normal,
	Running,
	Special,
};

// Enemy Hit Direction
UENUM(BlueprintType)
enum class EHitDirection : uint8
{
	None UMETA(DisplayName = "None"),
	Front UMETA(DisplayName = "Front"),
	Back  UMETA(DisplayName = "Back"),
	Left  UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right"),
};
