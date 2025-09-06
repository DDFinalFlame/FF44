
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
            W->SetGrabOpenerWindow(true);         // 그랩 오프너 창 ON
            if (bAlsoEnableNormalHitbox)
                W->BeginAttackWindow();            // 일반 히트박스도 ON
        }
}

void UANS_GrabOpenerWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    if (ACharacter* Boss = Cast<ACharacter>(MeshComp ? MeshComp->GetOwner() : nullptr))
        if (ABossMeleeWeapon* W = FindBossWeapon(Boss))
        {
            if (bAlsoEnableNormalHitbox)
                W->EndAttackWindow();             // 일반 히트박스 OFF
            W->SetGrabOpenerWindow(false);        // 그랩 오프너 창 OFF
            W->ResetGrabWindowTrigger();          // 창 닫을 때 1회 트리거 리셋
        }
}
