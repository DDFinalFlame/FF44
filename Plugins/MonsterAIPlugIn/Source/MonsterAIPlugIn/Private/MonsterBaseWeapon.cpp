// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterBaseWeapon.h"
#include "Components/PrimitiveComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "MonsterCharacter.h"

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
        // ���� Actor�� Destroy�� �� ���� ����
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
    if (!HasAuthority()) return; // ���������� ����
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
    //�α� Ȯ��
    if (Victim)
    {
        FString Msg = FString::Printf(TEXT("[Weapon] %s hit %s"),
            *GetName(), *Victim->GetName());

        // ȭ�� ����� ��� (���� �۾�, 2�� ����)
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,                 // Key (-1 = ���ο� �޽���)
                2.f,                // Duration (��)
                FColor::Red,        // Color
                Msg
            );
        }
    }
    //// 1) �̺�Ʈ�� HitReact ����
    //{
    //    FGameplayEventData Payload;
    //    Payload.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Hit"));
    //    Payload.Instigator = OwnerMonster; //���ͳ��� ���� X
    //    Payload.Target = Victim;
    //    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Victim, Payload.EventTag, Payload);
    //}

    //// 2) ������ GE ����(���⼭ ���� ü�� ����)
    //if (!DamageGE || !OwnerMonster) return;

    //UAbilitySystemComponent* SourceASC =
    //    UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerMonster);
    //UAbilitySystemComponent* TargetASC =
    //    UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Victim);

    //if (!SourceASC || !TargetASC) return;

    //// ���ؽ�Ʈ ���� + ������ ����(�߿�)
    //FGameplayEffectContextHandle Ctx = SourceASC->MakeEffectContext();
    //Ctx.AddInstigator(OwnerMonster, OwnerMonster->GetController());

    //// Spec ����
    //FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DamageGE, 1.f, Ctx);
    //if (!Spec.IsValid()) return;

    //// Ÿ�꿡 ����
    //TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

    if (!Victim || !OwnerMonster) return;

    UAbilitySystemComponent* SourceASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerMonster);
    if (!SourceASC) return;

    // �������� ���� ���ݷ� �б� (�ʿ��ϸ� ��� ���ϱ�)
    float Attack = SourceASC->GetNumericAttribute(UMonsterAttributeSet::GetAttackPowerAttribute());
    const float SkillCoeff = 1.0f;
    const float FinalAttack = Attack * SkillCoeff;

    // **�̺�Ʈ�� ���ݷ¸� ��� ����**
    FGameplayEventData Payload;
    Payload.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Hit"));
    Payload.Instigator = OwnerMonster;  
    Payload.Target = Victim;
    Payload.EventMagnitude = FinalAttack; // ���ݷ�

    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        Victim, Payload.EventTag, Payload);
}

