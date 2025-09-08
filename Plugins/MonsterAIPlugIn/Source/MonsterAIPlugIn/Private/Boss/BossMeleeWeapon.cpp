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
        bTriggeredThisWindow = false; // 창 닫힐 때 1회 트리거 리셋
    }
}

void ABossMeleeWeapon::ResetGrabWindowTrigger()
{
    bTriggeredThisWindow = false;
}

void ABossMeleeWeapon::ApplyHit(AActor* Victim, const FHitResult& Hit)
{
    Super::ApplyHit(Victim, Hit);

    // 2) 오프너 유효 프레임 + 이번 창에서 1회만 + 서버 권한
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
    // 3) 보스 GA 트리거 이벤트 한 발만 쏜다 (타깃은 이벤트 데이터로 전달)
    if (BossGrabTriggerTag.IsValid())
    {
        FGameplayEventData Data;
        Data.EventTag = BossGrabTriggerTag;
        Data.Instigator = Boss;
        Data.Target = Target;                 // GA에서 Victim으로 사용
        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Boss, BossGrabTriggerTag, Data);
    }

    if (bNotifyVictimOnSpecialHit)
    {
        FGameplayEventData Data;
        Data.EventTag = VictimSpecialHitTag;
        Data.Instigator = Boss;
        Data.Target = Target;
        // 필요하면 Data.EventMagnitude, InstigatorTags/TargetTags에 정보 추가 가능
        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Target, VictimSpecialHitTag, Data);
    }

    // 4) 이번 창에서 1회만
    bTriggeredThisWindow = true;
}