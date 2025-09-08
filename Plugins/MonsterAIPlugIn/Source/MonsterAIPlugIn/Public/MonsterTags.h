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
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dying);
    


    // �̺�Ʈ ���� ��ȣ
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Monster_Hit);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Monster_Death);

    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Player_Hit);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Player_Grabbed);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Player_Lift);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Assemble);


    //���� ����
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_MaxHealth);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Health);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_AttackPower);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_MoveSpeed);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Defense);

    //Cue �±�
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(GC_Impact_Hit);


    // ���� ���� Ability
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Boss_Summon);       // ��ȯ ����
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Boss_Channel);      // ĳ����/ä�θ�
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Boss_PhaseStart);   // ������ ���� Ʈ����
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Boss_Swing1);

    // ���� ���� State
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Boss_Invuln);         // ����
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Boss_Channeling);     // ĳ���� ��

    // ���� ���� Event
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Boss_PhaseStart);     // ������ ����
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Boss_PhaseEnd);       // ������ ����
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Boss_RockHit);       // ������ ����
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Minion_Died);         // ��ȯ�� ��� �˸�
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Boss_Land);         // �ٴ� ���� �̺�Ʈ
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Boss_P2_WeakPointDestroyed); //���� �Ѽ���
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Boss_LightingDamage); //���� �Ѽ���
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Boss_Grab_Trigger);   //�������� Grab ���� ��ȣ
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Player_Grab_Trigger);   //�÷��̾�� Grab ���� ��ȣ
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Player_Grab_AniStart);   //�÷��̾�� �ִϸ��̼� ���� ��ȣ
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Player_Grab_AniEnd);   //�÷��̾�� �ִϸ��̼� �� ��ȣ


    // ���� ���� Data (������)
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Boss_Phase);           // ���� ������
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Boss_SummonCount);     // ��ȯ�� ����
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Boss_Damaged);               //SetbyCaller��
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Drop_Damage);               //SetbyCaller��


    // ���� ���� Cue
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(GC_Boss_InvulnShield);     // ���� �� ���� ����Ʈ
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(GC_Boss_Summon);           // ��ȯ ����Ʈ
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(GC_Boss_ChannelCast);      // ĳ���� ���� ����Ʈ

}