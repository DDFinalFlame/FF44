#include "MonsterTags.h"

UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Ability_HitReact, "Ability.Monster.HitReact");
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Ability_Attack, "Ability.Monster.Attack");
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Ability_Death, "Ability.Monster.Dead");
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Ability_Assemble, "Ability.Monster.Assemble");

UE_DEFINE_GAMEPLAY_TAG(MonsterTags::State_HitReacting, "State.HitReacting");     
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::State_Dead, "State.Dead");
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::State_Attacking, "State.Attacking");
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::State_AttackRecover, "State.AttackRecover");
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::State_NoPawnCollision, "State.NoPawnCollision");
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::State_Stunned, "State.Stunned");
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::State_Assembling, "State.Assembling");
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::State_Dying, "State.Dying");

UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Event_Monster_Hit, "Event.Monster.Hit");
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Event_Monster_Death, "Event.Monster.Death");

UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Event_Player_Hit, "Event.Player.Hit");
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Event_Assemble, "Event.Assemble");

UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Data_MaxHealth, "Data.MaxHealth");
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Data_Health, "Data.Health");
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Data_AttackPower, "Data.AttackPower");
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Data_MoveSpeed, "Data.MoveSpeed");
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Data_Defense, "Data.Defense");

UE_DEFINE_GAMEPLAY_TAG(MonsterTags::GC_Impact_Hit, "GameplayCue.Impact.Hit");

//보스 관련 tag
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Ability_Boss_Summon, "Ability.Boss.Summon");        // 보스 소환 패턴 GA
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Ability_Boss_Channel, "Ability.Boss.Channel");      // 보스 캐스팅 루프 GA
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Ability_Boss_PhaseStart, "Ability.Boss.PhaseStart");// 페이즈 시작용 트리거 GA

UE_DEFINE_GAMEPLAY_TAG(MonsterTags::State_Boss_Invuln, "State.Boss.Invuln");   // 무적 상태
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::State_Boss_Channeling, "State.Boss.Channeling"); // 제자리 캐스팅 중

UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Event_Boss_PhaseStart, "Event.Boss.PhaseStart");   // 페이즈 시작 트리거
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Event_Minion_Died, "Event.Minion.Died");           // 소환몹 사망 → 보스 알림
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Event_Boss_PhaseEnd, "Event.Boss.PhaseEnd");       // 소환몹 전멸 후 보스 다음 패턴
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Event_Boss_Land, "Event.Boss.Land");		   // 보스 바닥 착지
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Event_Boss_P2_WeakPointDestroyed, "Event.Boss.Phase2.WeakPointDestroyed");

UE_DEFINE_GAMEPLAY_TAG(MonsterTags::GC_Boss_InvulnShield, "GameplayCue.Boss.InvulnShield"); // 무적시 쉴드/이펙트
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::GC_Boss_Summon, "GameplayCue.Boss.Summon");             // 소환 시 VFX/SFX
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::GC_Boss_ChannelCast, "GameplayCue.Boss.ChannelCast");   // 캐스팅 루프 비주얼

UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Data_Boss_Phase, "Data.Boss.Phase");          // 현재 페이즈 인덱스
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Data_Boss_SummonCount, "Data.Boss.SummonCount"); // 소환몹 수량
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Data_Damage, "Data.Boss.Damage");