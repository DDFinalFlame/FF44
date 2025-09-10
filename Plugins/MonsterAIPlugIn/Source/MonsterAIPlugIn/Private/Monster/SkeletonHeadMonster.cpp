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

    if (AbilitySystemComponent)
    {
        if (const UMonsterAttributeSet* Attr = Cast<UMonsterAttributeSet>(AttributeSet))
        {
            if (!bDeathSoundPlayed && Attr->GetHealth() <= 0.f)
            {
                bDeathSoundPlayed = true; // 중복 방지 플래그

                if (DeathSound)   // USoundBase* DeathSound = nullptr; (헤더에 UPROPERTY로 선언)
                {
                    UGameplayStatics::PlaySoundAtLocation(
                        this,
                        DeathSound,
                        GetActorLocation()
                    );
                }
            }
        }
    }

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
                    // === 벽(시야) 체크 추가 ===
                    FHitResult Hit;
                    FCollisionQueryParams Params(SCENE_QUERY_STAT(AssembleCheck), false, this);
                    Params.AddIgnoredActor(this);
                    Params.AddIgnoredActor(player);

                    bool bBlocked = GetWorld()->LineTraceSingleByChannel(
                        Hit,
                        GetActorLocation() + FVector(0, 0, 50),   // 몬스터 위치(조금 위로 보정)
                        player->GetActorLocation() + FVector(0, 0, 50), // 플레이어 위치(머리/가슴 높이 보정)
                        ECC_Visibility,   // 가시성 채널 사용
                        Params
                    );

                    if (!bBlocked) // 막는 게 없을 때만 발동
                    {
                        bAssembleRequested = true;

                        FGameplayEventData Ev;
                        Ev.EventTag = MonsterTags::Event_Assemble;
                        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, Ev.EventTag, Ev);

                        SetMonsterState(EMonsterState::Assembling);
                    }
                }
            }
        }
    }
}