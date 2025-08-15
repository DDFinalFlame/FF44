#pragma once

UENUM(BlueprintType)
enum class EAIBehavior : uint8
{
	Idle,
	Patrol,
	MeleeAttack,
	Approach,
	Investigate
};