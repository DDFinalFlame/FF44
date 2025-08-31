#include "Animation/ANS_MonsterAttackWindow.h"
#include "Monster/MonsterCharacter.h"
#include "Weapon/MonsterBaseWeapon.h"

void UANS_MonsterAttackWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
    if (!MeshComp) return;

    // ���� ĳ���� ã��
   
        //if (AMonsterBaseWeapon* W = MC->GetWeapon())
        //{
        //    W->BeginAttackWindow();   // ��Ʈ�ڽ� On (���������� �����ϵ��� ���� �ʿ� üũ ����)
        //}
        //else
        //{
        //    // ���� ���� ĳ���� ��Ʈ�ڽ� ���� �������:
        //    MC->BeginAttackWindow();  // ���û���(���� ����)
        //}
        if (AMonsterCharacter* MC = Cast<AMonsterCharacter>(MeshComp->GetOwner()))
        {
            MC->BeginAttackWindow();   // �� ������ ĳ���� ������
        }
    
}

void UANS_MonsterAttackWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;

    //if (AMonsterCharacter* MC = Cast<AMonsterCharacter>(MeshComp->GetOwner()))
    //{
    //    if (AMonsterBaseWeapon* W = MC->GetWeapon())
    //    {
    //        W->EndAttackWindow();     // ��Ʈ�ڽ� Off
    //    }
    //    else
    //    {
    //        MC->EndAttackWindow();    // ���û���
    //    }
    //}
    if (AMonsterCharacter* MC = Cast<AMonsterCharacter>(MeshComp->GetOwner()))
    {
        MC->EndAttackWindow();     // �� ������ ĳ���� ������
    }
}