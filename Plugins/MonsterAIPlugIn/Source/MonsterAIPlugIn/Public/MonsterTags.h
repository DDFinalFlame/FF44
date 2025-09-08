#pragma once
#include "NativeGameplayTags.h" 

namespace MonsterTags
{
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_HitReact);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Death);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Assemble);

    // 상태정보
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_HitReacting);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dead);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Attacking);      
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_AttackRecover);  
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_NoPawnCollision);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Stunned);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Assembling);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dying);
    


    // 이벤트 관련 신호
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Monster_Hit);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Monster_Death);

    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Player_Hit);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Player_Grabbed);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Player_Lift);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Assemble);


    //스텟 관련
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_MaxHealth);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Health);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_AttackPower);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_MoveSpeed);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Defense);

    //Cue 태그
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(GC_Impact_Hit);


    // 보스 전용 Ability
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Boss_Summon);       // 소환 패턴
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Boss_Channel);      // 캐스팅/채널링
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Boss_PhaseStart);   // 페이즈 시작 트리거
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Boss_Swing1);

    // 보스 전용 State
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Boss_Invuln);         // 무적
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Boss_Channeling);     // 캐스팅 중

    // 보스 전용 Event
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Boss_PhaseStart);     // 페이즈 시작
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Boss_PhaseEnd);       // 페이즈 종료
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Boss_RockHit);       // 페이즈 종료
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Minion_Died);         // 소환몹 사망 알림
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Boss_Land);         // 바닥 착지 이벤트
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Boss_P2_WeakPointDestroyed); //석상 뿌셔짐
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Boss_LightingDamage); //석상 뿌셔짐
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Boss_Grab_Trigger);   //보스에게 Grab 시작 신호
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Player_Grab_Trigger);   //플레이어에게 Grab 시작 신호
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Player_Grab_AniStart);   //플레이어에게 애니메이션 시작 신호
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Player_Grab_AniEnd);   //플레이어에게 애니메이션 끝 신호


    // 보스 전용 Data (선택적)
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Boss_Phase);           // 현재 페이즈
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Boss_SummonCount);     // 소환몹 수량
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Boss_Damaged);               //SetbyCaller용
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Drop_Damage);               //SetbyCaller용


    // 보스 전용 Cue
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(GC_Boss_InvulnShield);     // 무적 시 쉴드 이펙트
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(GC_Boss_Summon);           // 소환 이펙트
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(GC_Boss_ChannelCast);      // 캐스팅 루프 이펙트

}