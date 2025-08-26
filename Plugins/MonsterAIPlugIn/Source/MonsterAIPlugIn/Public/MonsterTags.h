#pragma once
#include "NativeGameplayTags.h" 

namespace MonsterTags
{
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_HitReact);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Death);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Assemble);

    // ��������
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_HitReacting);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dead);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Attacking);      
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_AttackRecover);  
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_NoPawnCollision);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Stunned);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Assembling);


    // �̺�Ʈ ���� ��ȣ
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Monster_Hit);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Hit);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Death);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Assemble);


    //���� ����
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_MaxHealth);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Health);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_AttackPower);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_MoveSpeed);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Defense);

    //Cue �±�
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(GC_Impact_Hit);
}