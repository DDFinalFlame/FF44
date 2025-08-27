#include "Animation/ANS_MonsterAttackWindow.h"
#include "Monster/MonsterCharacter.h"
#include "Weapon/MonsterBaseWeapon.h"

void UANS_MonsterAttackWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
    if (!MeshComp) return;

    // 몬스터 캐릭터 찾기
    if (AMonsterCharacter* MC = Cast<AMonsterCharacter>(MeshComp->GetOwner()))
    {
        if (AMonsterBaseWeapon* W = MC->GetWeapon())
        {
            W->BeginAttackWindow();   // 히트박스 On (서버에서만 동작하도록 무기 쪽에 체크 있음)
        }
        else
        {
            // 무기 없이 캐릭터 히트박스 쓰는 구조라면:
            MC->BeginAttackWindow();  // 선택사항(있을 때만)
        }
    }
}

void UANS_MonsterAttackWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;

    if (AMonsterCharacter* MC = Cast<AMonsterCharacter>(MeshComp->GetOwner()))
    {
        if (AMonsterBaseWeapon* W = MC->GetWeapon())
        {
            W->EndAttackWindow();     // 히트박스 Off
        }
        else
        {
            MC->EndAttackWindow();    // 선택사항
        }
    }
}