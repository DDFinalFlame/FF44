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
}