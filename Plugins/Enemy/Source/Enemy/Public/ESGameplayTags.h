#pragma once

#include "NativeGameplayTags.h"

namespace SLGameplayTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Attack_Normal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Attack_Running);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Attack_Special);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_State_Attack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_State_Hit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_State_Patrol);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Boss_Attack_Melee1);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Boss_Attack_Melee2);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Boss_Attack_Melee3);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Boss_Attack_Summon);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Boss_Attack_Grab);


}