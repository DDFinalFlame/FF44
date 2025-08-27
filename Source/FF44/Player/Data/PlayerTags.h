#pragma once
#include "NativeGameplayTags.h"

namespace PlayerTags
{
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_HitReact);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_Dodge);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_Attack);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_Death);

    // 상태정보
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Player_Weapon_Equip);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Player_Weapon_UnEquip);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Player_HitReacting);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Player_Dodge);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Player_Attack);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Player_Dead);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Player_Combo_1);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Player_Combo_2);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Player_Combo_3);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Player_Combo_Enable);

    // 이벤트 관련 신호
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Player_Hit);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Player_Death);

    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Monster_Hit);
}