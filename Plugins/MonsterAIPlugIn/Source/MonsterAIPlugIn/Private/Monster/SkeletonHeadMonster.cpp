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

    if (AbilitySystemComponent)
    {
        if (const UMonsterAttributeSet* Attr = Cast<UMonsterAttributeSet>(AttributeSet))
        {
            if (!bDeathSoundPlayed && Attr->GetHealth() <= 0.f)
            {
                bDeathSoundPlayed = true; // �ߺ� ���� �÷���

                if (DeathSound)   // USoundBase* DeathSound = nullptr; (����� UPROPERTY�� ����)
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
                    // === ��(�þ�) üũ �߰� ===
                    FHitResult Hit;
                    FCollisionQueryParams Params(SCENE_QUERY_STAT(AssembleCheck), false, this);
                    Params.AddIgnoredActor(this);
                    Params.AddIgnoredActor(player);

                    bool bBlocked = GetWorld()->LineTraceSingleByChannel(
                        Hit,
                        GetActorLocation() + FVector(0, 0, 50),   // ���� ��ġ(���� ���� ����)
                        player->GetActorLocation() + FVector(0, 0, 50), // �÷��̾� ��ġ(�Ӹ�/���� ���� ����)
                        ECC_Visibility,   // ���ü� ä�� ���
                        Params
                    );

                    if (!bBlocked) // ���� �� ���� ���� �ߵ�
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