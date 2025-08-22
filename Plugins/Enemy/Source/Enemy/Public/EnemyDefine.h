#pragma once

UENUM(BlueprintType)
enum class EAIBehavior : uint8
{
	Idle,
	Patrol,
	MeleeAttack,
	RangeAttack,
	Approach,
	Investigate,
	Hit
};

UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	Iron,
	Archer
};

FName GetEnemyRowName(EEnemyType CharacterType);

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	LeftHand,
	RightHand,
	Bow,
};
