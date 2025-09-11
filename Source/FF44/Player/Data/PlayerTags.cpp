#include "PlayerTags.h"

UE_DEFINE_GAMEPLAY_TAG(PlayerTags::Ability_Player_HitReact, "Ability.Player.HitReact");
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::Ability_Player_Dodge, "Ability.Player.Dodge");
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::Ability_Player_Attack, "Ability.Player.Attack");
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::Ability_Player_Attack_KeyDown, "Ability.Player.Attack.KeyDown");
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::Ability_Player_Potion, "Ability.Player.Potion");	// 삭제
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::Ability_Player_UseItem, "Ability.Player.UseItem");
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::Ability_Player_Equip, "Ability.Player.Equip");
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::Ability_Player_Death, "Ability.Player.Death");

UE_DEFINE_GAMEPLAY_TAG(PlayerTags::State_Player_Move_Walk, "State.Player.Move.Walk");
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::State_Player_Move_Run, "State.Player.Move.Run");
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::State_Player_Weapon_ChangeEquip, "State.Player.Weapon.ChangeEquip");
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::State_Player_Weapon_Equip, "State.Player.Weapon.Equip");
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::State_Player_Weapon_UnEquip, "State.Player.Weapon.UnEquip");
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::State_Player_HitReacting, "State.Player.HitReacting");
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::State_Player_HitReacting_Special, "State.Player.HitReacting.Special");
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::State_Player_Dodge, "State.Player.Dodge");
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::State_Player_Dead, "State.Player.Dead");
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::State_Player_Attack, "State.Player.Attack");
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::State_Player_KeyDownAttack, "State.Player.Attack.KeyDown");
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::State_Player_Combo_1, "State.Player.Attack.Combo.1");
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::State_Player_Combo_2, "State.Player.Attack.Combo.2");
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::State_Player_Combo_3, "State.Player.Attack.Combo.3");
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::State_Player_Combo_Enable, "State.Player.Attack.Combo.Enable");
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::State_Player_Drink, "State.Player.Drink");	// 삭제
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::State_Player_ItemUse, "State.Player.UseItem");
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::State_Player_ItemUse_Potion, "State.Player.UseItem.Potion");

UE_DEFINE_GAMEPLAY_TAG(PlayerTags::Event_Player_Hit, "Event.Player.Hit");
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::Event_Player_Grabbed, "Event.Player.Grabbed");
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::Event_Player_GrabTrigger, "Event.Player.GrabTrigger");
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::Event_Player_GrabAniStart, "Event.Player.GrabAniStart");
UE_DEFINE_GAMEPLAY_TAG(PlayerTags::Event_Player_Death, "Event.Player.Death");

UE_DEFINE_GAMEPLAY_TAG(PlayerTags::Event_Monster_Hit, "Event.Monster.Hit");

UE_DEFINE_GAMEPLAY_TAG(PlayerTags::Stat_Player_Stamina_RegenRate, "Stat.Player.Stamina.RegenRate");