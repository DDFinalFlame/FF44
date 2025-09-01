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

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Boss_Attack_Melee);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Boss_Attack_Summon);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Boss_Attack_Recall);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Boss_Attack_Grab);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Event_SummonStart);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Event_Recall);


}