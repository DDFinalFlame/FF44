
#include "Animation/ANS_GrabOpenerWindow.h"
#include "Boss/BossMeleeWeapon.h"
#include "Weapon/MonsterBaseWeapon.h"
#include "GameFramework/Character.h"

static ABossMeleeWeapon* FindBossWeapon(ACharacter* Owner)
{
    if (!Owner) return nullptr;
    TArray<AActor*> Attached;
    Owner->GetAttachedActors(Attached);
    for (AActor* A : Attached)
        if (auto* W = Cast<ABossMeleeWeapon>(A)) return W;
    return nullptr;
}

void UANS_GrabOpenerWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    if (ACharacter* Boss = Cast<ACharacter>(MeshComp ? MeshComp->GetOwner() : nullptr))
        if (ABossMeleeWeapon* W = FindBossWeapon(Boss))
        {
            W->SetGrabOpenerWindow(true);         // �׷� ������ â ON
            if (bAlsoEnableNormalHitbox)
                W->BeginAttackWindow();            // �Ϲ� ��Ʈ�ڽ��� ON
        }
}

void UANS_GrabOpenerWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    if (ACharacter* Boss = Cast<ACharacter>(MeshComp ? MeshComp->GetOwner() : nullptr))
        if (ABossMeleeWeapon* W = FindBossWeapon(Boss))
        {
            if (bAlsoEnableNormalHitbox)
                W->EndAttackWindow();             // �Ϲ� ��Ʈ�ڽ� OFF
            W->SetGrabOpenerWindow(false);        // �׷� ������ â OFF
            W->ResetGrabWindowTrigger();          // â ���� �� 1ȸ Ʈ���� ����
        }
}
