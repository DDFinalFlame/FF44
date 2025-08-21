#include "EnemyDefine.h"

FName GetEnemyRowName(EEnemyType CharacterType)
{
	{
		switch (CharacterType)
		{
		case EEnemyType::Iron: return FName("Iron");
		case EEnemyType::Archer:  return FName("Archer");
		}
		return FName("Default");
	}
}
