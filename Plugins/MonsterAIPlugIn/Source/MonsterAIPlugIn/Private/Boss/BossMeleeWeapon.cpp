// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss/BossMeleeWeapon.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "AIController.h"                            
#include "BehaviorTree/BlackboardComponent.h"        
#include "AbilitySystemBlueprintLibrary.h"           
#include "GameplayEffect.h"    
#include "AbilitySystemComponent.h"


ABossMeleeWeapon::ABossMeleeWeapon()
{
    Hitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("Hitbox"));
    RootComponent = Hitbox;
    Hitbox->InitBoxExtent(BoxExtent);


    RegisterHitbox(Hitbox);
}

void ABossMeleeWeapon::SetGrabOpenerWindow(bool bOpen)
{
    bGrabOpenerWindow = bOpen;
    if (!bOpen)
    {
        bTriggeredThisWindow = false; // â ���� �� 1ȸ Ʈ���� ����
    }
}

void ABossMeleeWeapon::ResetGrabWindowTrigger()
{
    bTriggeredThisWindow = false;
}

void ABossMeleeWeapon::ApplyHit(AActor* Victim, const FHitResult& Hit)
{
    Super::ApplyHit(Victim, Hit);

    // 2) ������ ��ȿ ������ + �̹� â���� 1ȸ�� + ���� ����
    if (!bGrabOpenerWindow || bTriggeredThisWindow) return;
    if (!HasAuthority()) return;

    ACharacter* Boss = Cast<ACharacter>(GetOwner());
    ACharacter* Target = Cast<ACharacter>(Victim);
    if (!Boss || !Target) return;

    if (BlockVictimTags.Num() > 0)
    {
        if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target))
        {
            if (TargetASC->HasAnyMatchingGameplayTags(BlockVictimTags))
            {
                return;
            }
        }
    }
    // 3) ���� GA Ʈ���� �̺�Ʈ �� �߸� ��� (Ÿ���� �̺�Ʈ �����ͷ� ����)
    if (BossGrabTriggerTag.IsValid())
    {
        FGameplayEventData Data;
        Data.EventTag = BossGrabTriggerTag;
        Data.Instigator = Boss;
        Data.Target = Target;                 // GA���� Victim���� ���
        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Boss, BossGrabTriggerTag, Data);
    }

    if (bNotifyVictimOnSpecialHit)
    {
        FGameplayEventData Data;
        Data.EventTag = VictimSpecialHitTag;
        Data.Instigator = Boss;
        Data.Target = Target;
        // �ʿ��ϸ� Data.EventMagnitude, InstigatorTags/TargetTags�� ���� �߰� ����
        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Target, VictimSpecialHitTag, Data);
    }

    // 4) �̹� â���� 1ȸ��
    bTriggeredThisWindow = true;
}