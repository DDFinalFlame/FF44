#include "ESGameplayTags.h"

namespace SLGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Attack_Normal, "Enemy.Attack.Normal");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Attack_Running, "Enemy.Attack.Running");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Attack_Special, "Enemy.Attack.Special");


	UE_DEFINE_GAMEPLAY_TAG(Enemy_State_Attack, "Enemy.State.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_State_Hit, "Enemy.State.Hit");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_State_Patrol, "Enemy.State.Patrol");

	UE_DEFINE_GAMEPLAY_TAG(Enemy_Boss_Attack_Melee, "Enemy.Boss.Attack.Melee");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Boss_Attack_Summon, "Enemy.Boss.Attack.Summon");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Boss_Attack_Recall, "Enemy.Boss.Attack.Recall");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Boss_Attack_Grab, "Enemy.Boss.Attack.Grab");

	UE_DEFINE_GAMEPLAY_TAG(Enemy_Event_SummonStart, "Enemy.Event.SummonStart");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Event_Recall, "Enemy.Event.Recall");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Event_Buff, "Enemy.Event.Buff");

}