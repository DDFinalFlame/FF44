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

//���� ���� tag
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Ability_Boss_Summon, "Ability.Boss.Summon");        // ���� ��ȯ ���� GA
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Ability_Boss_Channel, "Ability.Boss.Channel");      // ���� ĳ���� ���� GA
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Ability_Boss_PhaseStart, "Ability.Boss.PhaseStart");// ������ ���ۿ� Ʈ���� GA

UE_DEFINE_GAMEPLAY_TAG(MonsterTags::State_Boss_Invuln, "State.Boss.Invuln");   // ���� ����
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::State_Boss_Channeling, "State.Boss.Channeling"); // ���ڸ� ĳ���� ��

UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Event_Boss_PhaseStart, "Event.Boss.PhaseStart");   // ������ ���� Ʈ����
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Event_Minion_Died, "Event.Minion.Died");           // ��ȯ�� ��� �� ���� �˸�
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Event_Boss_PhaseEnd, "Event.Boss.PhaseEnd");       // ��ȯ�� ���� �� ���� ���� ����
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Event_Boss_Land, "Event.Boss.Land");		   // ���� �ٴ� ����
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Event_Boss_P2_WeakPointDestroyed, "Event.Boss.Phase2.WeakPointDestroyed");

UE_DEFINE_GAMEPLAY_TAG(MonsterTags::GC_Boss_InvulnShield, "GameplayCue.Boss.InvulnShield"); // ������ ����/����Ʈ
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::GC_Boss_Summon, "GameplayCue.Boss.Summon");             // ��ȯ �� VFX/SFX
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::GC_Boss_ChannelCast, "GameplayCue.Boss.ChannelCast");   // ĳ���� ���� ���־�

UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Data_Boss_Phase, "Data.Boss.Phase");          // ���� ������ �ε���
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Data_Boss_SummonCount, "Data.Boss.SummonCount"); // ��ȯ�� ����
UE_DEFINE_GAMEPLAY_TAG(MonsterTags::Data_Damage, "Data.Boss.Damage");