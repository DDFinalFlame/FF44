// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/MonsterBaseWeapon.h"
#include "Components/PrimitiveComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Monster/MonsterCharacter.h"
#include "MonsterTags.h"

// Sets default values
AMonsterBaseWeapon::AMonsterBaseWeapon()
{
    bReplicates = true;
    SetReplicateMovement(true);
    PrimaryActorTick.bCanEverTick = false;
}

void AMonsterBaseWeapon::Init(AMonsterCharacter* InOwner, USkeletalMeshComponent* AttachTo, FName Socket)
{
    OwnerMonster = InOwner;
    SetOwner(InOwner);

    if (AttachTo)
    {
        FAttachmentTransformRules Rules(EAttachmentRule::SnapToTarget, true);
        AttachToComponent(AttachTo, Rules, Socket);
    }

    if (InOwner)
    {
        // 몬스터 Actor가 Destroy될 때 같이 정리
        InOwner->OnDestroyed.AddDynamic(this, &AMonsterBaseWeapon::OnOwnerDestroyed);
    }
}

void AMonsterBaseWeapon::OnOwnerDestroyed(AActor* DestroyedActor)
{
    Destroy();
}

void AMonsterBaseWeapon::RegisterHitbox(UPrimitiveComponent* Comp)
{
    if (!Comp) return;
    Hitboxes.Add(Comp);

    Comp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Comp->SetCollisionObjectType(ECC_WorldDynamic);
    Comp->SetCollisionResponseToAllChannels(ECR_Ignore);
    Comp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    Comp->SetGenerateOverlapEvents(false);

    Comp->OnComponentBeginOverlap.AddDynamic(this, &AMonsterBaseWeapon::OnHitboxBeginOverlap);
}

void AMonsterBaseWeapon::BeginAttackWindow()
{
    if (!HasAuthority()) return; // 서버에서만 판정
    bActive = true;
    HitActorsThisSwing.Reset();
    SetHitboxEnable(true);

    //UE_LOG(LogTemp, Warning, TEXT("Hitbox enable"));
}

void AMonsterBaseWeapon::EndAttackWindow()
{
    if (!HasAuthority()) return;
    SetHitboxEnable(false);
    bActive = false;
    HitActorsThisSwing.Reset();

   //UE_LOG(LogTemp, Warning, TEXT("Hitbox disable"));
}

void AMonsterBaseWeapon::SetHitboxEnable(bool bEnable)
{
    for (auto* C : Hitboxes)
        if (C) C->SetGenerateOverlapEvents(bEnable);
}

void AMonsterBaseWeapon::OnHitboxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bActive || !HasAuthority()) return;
    if (!OtherActor || OtherActor == OwnerMonster) return;
    if (HitActorsThisSwing.Contains(OtherActor)) return;
    if (OtherActor->IsA<AMonsterCharacter>()) return;

    HitActorsThisSwing.Add(OtherActor);
    ApplyHit(OtherActor, SweepResult);
}

void AMonsterBaseWeapon::ApplyHit(AActor* Victim, const FHitResult& Hit)
{
    //로그 확인
    if (Victim)
    {
        FString Msg = FString::Printf(TEXT("[Weapon] %s hit %s"),
            *GetName(), *Victim->GetName());

        // 화면 디버그 출력 (빨간 글씨, 2초 유지)
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,                 // Key (-1 = 새로운 메시지)
                2.f,                // Duration (초)
                FColor::Red,        // Color
                Msg
            );
        }
    }
    // 1) 이벤트로 HitReact 유도
    {
        FGameplayEventData Payload;
        Payload.EventTag = MonsterTags::Event_Player_Hit;
        Payload.Instigator = OwnerMonster; //몬스터끼리 공격 X
        Payload.Target = Victim;
        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Victim, Payload.EventTag, Payload);
    }

}

