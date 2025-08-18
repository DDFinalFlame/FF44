#include "MonsterTags.h"

UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Ability_HitReact, "Ability.Monster.HitReact");
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Ability_Attack, "Ability.Monster.Attack");
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Ability_Death, "Ability.Monster.Dead");

UE_DEFINE_GAMEPLAY_TAG(MonsterTags::State_HitReacting, "State.HitReacting");     
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::State_Dead, "State.Dead");

UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Event_Hit, "Event.Hit");
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Event_Death, "Event.Death");

UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Data_MaxHealth, "Data.MaxHealth");
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Data_Health, "Data.Health");
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Data_AttackPower, "Data.AttackPower");
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Data_MoveSpeed, "Data.MoveSpeed");