#include "Animation/ANS_MonsterAttackWindow.h"
#include "Monster/MonsterCharacter.h"
#include "Weapon/MonsterBaseWeapon.h"

void UANS_MonsterAttackWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
    if (!MeshComp) return;


        if (AMonsterCharacter* MC = Cast<AMonsterCharacter>(MeshComp->GetOwner()))
        {
            MC->BeginAttackWindow();  
        }
    
}

void UANS_MonsterAttackWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;


    if (AMonsterCharacter* MC = Cast<AMonsterCharacter>(MeshComp->GetOwner()))
    {
        MC->EndAttackWindow();    
    }
}