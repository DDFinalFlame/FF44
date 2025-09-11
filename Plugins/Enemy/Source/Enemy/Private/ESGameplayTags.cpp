#include "ESGameplayTags.h"

namespace SLGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Attack_Normal, "Enemy.Attack.Normal");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Attack_Running, "Enemy.Attack.Running");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Attack_Special, "Enemy.Attack.Special");

	UE_DEFINE_GAMEPLAY_TAG(Enemy_State_Intro, "Enemy.State.Intro");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_State_Attack, "Enemy.State.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_State_Hit, "Enemy.State.Hit");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_State_Patrol, "Enemy.State.Patrol");

	UE_DEFINE_GAMEPLAY_TAG(Enemy_Boss_Attack_Melee, "Enemy.Boss.Attack.Melee");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Boss_Attack_Summon, "Enemy.Boss.Attack.Summon");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Boss_Attack_Recall, "Enemy.Boss.Attack.Recall");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Boss_Attack_Grab, "Enemy.Boss.Attack.Grab");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Boss_Attack_EvadeStart, "Enemy.Boss.Attack.EvadeStart");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Boss_Attack_EvadeEnd, "Enemy.Boss.Attack.EvadeEnd");

	UE_DEFINE_GAMEPLAY_TAG(Enemy_Event_IntroStart, "Enemy.Event.IntroStart");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Event_SummonStart, "Enemy.Event.SummonStart");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Event_Recall, "Enemy.Event.Recall");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Event_Buff, "Enemy.Event.Buff");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Event_EvadeEnd, "Enemy.Event.EvadeEnd");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Event_EndAbility, "Enemy.Event.EndAbility");

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Sevarog_DuringEvade, "GameplayCue.Sevarog.DuringEvade");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Sevarog_StartEvade, "GameplayCue.Sevarog.StartEvade");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Sevarog_EndEvade, "GameplayCue.Sevarog.EndEvade");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Sevarog_Buff, "GameplayCue.Sevarog.Buff");


}