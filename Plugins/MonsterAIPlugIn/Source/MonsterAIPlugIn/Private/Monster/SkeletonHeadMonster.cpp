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

    // 1) 이 몬스터만 레그돌 상태로 시작
    EnterRagdollState();               // (베이스의 유틸 사용)
    bAssembleRequested = false;

    // 2) 이 몬스터에만 Assemble GA 지급
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
        // 필요 시 Damage도 여기서 덮어쓰기 가능: Weapon->Damage = 12.f;
    }
}


void ASkeletonHeadMonster::Tick(float _dt)
{
    Super::Tick(_dt);

    if (!HasAuthority()) return;

    // 3) 플레이어가 임계 거리 이하면 이벤트로 GA 발동
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
                    Ev.EventTag = MonsterTags::Event_Assemble;   // GA가 이 태그로 트리거됨
                    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, Ev.EventTag, Ev);

                    SetMonsterState(EMonsterState::Assembling);   // 표시용(선택)
                }
            }
        }
    }
}