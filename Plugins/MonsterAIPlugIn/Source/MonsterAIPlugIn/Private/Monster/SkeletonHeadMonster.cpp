#include "Monster/SkeletonHeadMonster.h"
#include "Weapon/MonsterBaseWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GAS/GA_MonsterAssemble.h"
#include "MonsterTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"

ASkeletonHeadMonster::ASkeletonHeadMonster()
{
    PrimaryActorTick.bCanEverTick = true;

    AssembleAbilityClass = UGA_MonsterAssemble::StaticClass();
    AssembleTriggerDistance = 1200.f;
}

void ASkeletonHeadMonster::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority()) return;
    if (!WeaponClass || Weapon) return;

    // 1) �� ���͸� ���׵� ���·� ����
    EnterRagdollState();               // (���̽��� ��ƿ ���)
    bAssembleRequested = false;

    // 2) �� ���Ϳ��� Assemble GA ����
    if (AbilitySystemComponent && AssembleAbilityClass)
    {
        FGameplayAbilitySpec Spec(AssembleAbilityClass, 1);
        AbilitySystemComponent->GiveAbility(Spec);
    }


    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    Weapon = GetWorld()->SpawnActor<AMonsterBaseWeapon>(WeaponClass, FTransform::Identity, Params);
    if (Weapon)
    {
        Weapon->Init(this, GetMesh(), WeaponAttachSocketName);
        // �ʿ� �� Damage�� ���⼭ ����� ����: Weapon->Damage = 12.f;
    }
}


void ASkeletonHeadMonster::Tick(float _dt)
{
    Super::Tick(_dt);

    if (!HasAuthority()) return;

    // 3) �÷��̾ �Ӱ� �Ÿ� ���ϸ� �̺�Ʈ�� GA �ߵ�
    if (GetMonsterState() == EMonsterState::Ragdoll && !bAssembleRequested)
    {
        if (AssembleTriggerDistance > 0.f && AbilitySystemComponent)
        {
            ACharacter* player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
            if (player)
            {
                float d = FVector::Dist(player->GetActorLocation(), GetActorLocation());
                if (d <= AssembleTriggerDistance)
                {
                    bAssembleRequested = true;

                    FGameplayEventData Ev;
                    Ev.EventTag = MonsterTags::Event_Assemble;   // GA�� �� �±׷� Ʈ���ŵ�
                    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, Ev.EventTag, Ev);

                    SetMonsterState(EMonsterState::Assembling);   // ǥ�ÿ�(����)
                }
            }
        }
    }
}