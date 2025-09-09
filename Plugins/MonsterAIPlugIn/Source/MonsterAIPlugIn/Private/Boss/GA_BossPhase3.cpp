// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss/GA_BossPhase3.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "MonsterTags.h"
#include "MonsterAttributeSet.h"
#include "Boss/WeakPointActor.h"
#include "Monster/MonsterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemGlobals.h"
static bool ProjectToGround_NoTilt(
    UWorld* World,
    const FVector& XY,                   // X,Y�� �ǹ� ����
    FVector& OutGroundLoc,               // ���� ���� ��ġ(Z Ȯ��)
    float TraceUp = 1500.f,
    float TraceDown = 4000.f,
    float GroundOffset = 2.f,
    ECollisionChannel GroundChannel = ECC_Visibility,
    const FCollisionQueryParams* InParams = nullptr
)
{
    if (!World) return false;

    FCollisionQueryParams Params = InParams ? *InParams : FCollisionQueryParams(SCENE_QUERY_STAT(P3_WeakGround), false);
    const FVector Start = FVector(XY.X, XY.Y, XY.Z + TraceUp);
    const FVector End = FVector(XY.X, XY.Y, XY.Z - TraceDown);

    FHitResult Hit;
    if (World->LineTraceSingleByChannel(Hit, Start, End, GroundChannel, Params))
    {
        OutGroundLoc = Hit.ImpactPoint + FVector(0, 0, GroundOffset);
        return true;
    }
    return false;
}

UGA_BossPhase3::UGA_BossPhase3()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

    // �ʿ�� ����/���� �±� ���� ����
    // ActivationBlockedTags.AddTag(MonsterTags::State_Dying);
    // ActivationBlockedTags.AddTag(MonsterTags::State_Dead);
}

/* ============================ Ability lifecycle ============================ */

void UGA_BossPhase3::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, /*bRep*/true, /*bWasCancelled*/true);
        return;
    }

    ApplyInvuln();
    BindHP();

    AActor* Boss = GetAvatarActorFromActorInfo();
    if (!Boss)
    {
        K2_EndAbility();
        return;
    }

    UWorld* W = Boss->GetWorld();
    if (!W)
    {
        K2_EndAbility();
        return;
    }

    // 1) ���� ����
    W->GetTimerManager().SetTimer(RockTimer, this, &UGA_BossPhase3::Tick_Rock, RockInterval, /*bLoop*/true, /*FirstDelay*/0.f);

    // 2) ���� ���� (���� ����)
    const float FirstAtkDelay = FMath::FRandRange(AttackIntervalMin, AttackIntervalMax);
    W->GetTimerManager().SetTimer(AttackTimer, this, &UGA_BossPhase3::Tick_Attack, FirstAtkDelay, /*bLoop*/false);

    // 3) �̴Ͼ� ����
    W->GetTimerManager().SetTimer(MinionTimer, this, &UGA_BossPhase3::Tick_Minion, MinionInterval, /*bLoop*/true, /*FirstDelay*/2.f);

    // ���� �ı� �̺�Ʈ ���� (Phase2�� ���� �±� ���)
    if (UAbilityTask_WaitGameplayEvent* Wait =
        UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, MonsterTags::Event_Boss_P2_WeakPointDestroyed, nullptr, false, true))
    {
        Wait->EventReceived.AddDynamic(this, &UGA_BossPhase3::OnWeakPointDestroyedEvent);
        Wait->ReadyForActivation();
    }

    if (auto* WaitMinion = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this, MonsterTags::Event_Minion_Died, nullptr, /*OnlyOnce*/false, /*Exact*/true))
    {
        WaitMinion->EventReceived.AddDynamic(this, &UGA_BossPhase3::OnMinionDied);
        WaitMinion->ReadyForActivation();
    }
}

void UGA_BossPhase3::EndAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    if (ACharacter* Boss = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
    {
        FTimerManager& TM = Boss->GetWorldTimerManager();
        TM.ClearTimer(RockTimer);
        TM.ClearTimer(AttackTimer);
        TM.ClearTimer(MinionTimer);

        if (AAIController* AI = Cast<AAIController>(Boss->GetController()))
        {
            if (MoveFinishedHandle.IsValid())
            {
                if (UPathFollowingComponent* PFC = AI->GetPathFollowingComponent())
                {
                    PFC->OnRequestFinished.Remove(MoveFinishedHandle);
                }
                MoveFinishedHandle.Reset();
            }
            AI->StopMovement();
        }
    }

    if (ACharacter* C = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
    {
        if (UAnimInstance* Anim = C->GetMesh() ? C->GetMesh()->GetAnimInstance() : nullptr)
        {
            Anim->StopAllMontages(0.1f);
        }
    }

    RemoveInvuln();
    UnbindHP();

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

/* ============================ Helpers ============================ */

static FVector RandomInAnnulus2D(const FVector& Center, float Rmin, float Rmax)
{
    const float r = FMath::Sqrt(FMath::FRandRange(Rmin * Rmin, Rmax * Rmax));
    const float th = FMath::FRandRange(0.f, 2.f * PI);
    return Center + FVector(r * FMath::Cos(th), r * FMath::Sin(th), 0.f);
}

/* ============================ Loops ============================ */

void UGA_BossPhase3::Tick_Rock()
{
    ACharacter* Boss = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Boss || !FallingRockClass) return;

    const FVector XY = RandomInAnnulus2D(Boss->GetActorLocation(), RockRadiusMin, RockRadiusMax);
    const FVector SpawnLoc = XY + FVector(0, 0, 1200.f);            // ������ ����
    Boss->GetWorld()->SpawnActor<AActor>(FallingRockClass, FTransform(SpawnLoc));
}

void UGA_BossPhase3::Tick_Attack()
{
    ACharacter* Boss = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Boss) return;

    if (bAttackPlaying)
    {
        // �̹� ��� ���̸� ���� ���ุ
        const float Delay = FMath::FRandRange(AttackIntervalMin, AttackIntervalMax);
        Boss->GetWorldTimerManager().SetTimer(AttackTimer, this, &UGA_BossPhase3::Tick_Attack, Delay, /*bLoop*/false);
        return;
    }

    StartMoveToTargetOrAttack();
}

void UGA_BossPhase3::Tick_Minion()
{
    ACharacter* Boss = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Boss || !MinionClass) return;

    UWorld* World = Boss->GetWorld();
    if (!World || !Boss->HasAuthority()) return;

    for (int32 i = 0; i < MinionSpawnCountEachTime; ++i)
    {
        FVector XY = RandomInAnnulus2D(Boss->GetActorLocation(), 800.f, 1400.f);
        FVector SpawnLoc = XY; SpawnLoc.Z = Boss->GetActorLocation().Z + 50.f;
        const FTransform SpawnTM(FRotator::ZeroRotator, SpawnLoc);


        // Deferred Spawn���� BeginPlay ���� �����ϰ� ����
        ACharacter* Spawned = World->SpawnActorDeferred<ACharacter>(
            MinionClass, SpawnTM, /*Owner=*/Boss, /*Instigator=*/Boss,
            ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

        if (!Spawned) continue;

        if (AMonsterCharacter* MC = Cast<AMonsterCharacter>(Spawned))
        {
            MC->SetOwnerBoss(Boss); //  ���� ����!
        }

        UGameplayStatics::FinishSpawningActor(Spawned, SpawnTM);
        
        PhaseMinions.Add(Spawned);
    }
}

/* ============================ Montage done ============================ */

void UGA_BossPhase3::OnSmashMontageFinished()
{
    bAttackPlaying = false;

    if (AActor* B = GetAvatarActorFromActorInfo())
    {
        const float Delay = FMath::FRandRange(AttackIntervalMin, AttackIntervalMax);
        B->GetWorldTimerManager().SetTimer(AttackTimer, this, &UGA_BossPhase3::Tick_Attack, Delay, /*bLoop*/false);
    }
}

/* ============================ Weakpoint destroyed ============================ */

void UGA_BossPhase3::OnWeakPointDestroyedEvent(FGameplayEventData Payload)
{
    // Phase2�� �����ϰ� ���� �ı� �� ���� HP ���� ó��
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        float Damage = WeakPointDamageToBoss;
        if (Payload.EventMagnitude > 0.f)
        {
            Damage = Payload.EventMagnitude;
        }

        if (GE_WeakPointDamageToBoss)
        {
            FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
            AActor* BossActor = GetAvatarActorFromActorInfo();

            const AActor* InstigatorConst = Payload.Instigator.Get();
            AActor* EffectCauser = InstigatorConst ? const_cast<AActor*>(InstigatorConst) : nullptr;

            Ctx.AddInstigator(BossActor, EffectCauser);
            FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(GE_WeakPointDamageToBoss, 1.f, Ctx);
            if (Spec.IsValid())
            {
                Spec.Data->SetSetByCallerMagnitude(MonsterTags::Data_Boss_Damaged, Damage);
                ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
            }
        }
    }
}

/* ============================ HP watch & Invuln ============================ */

void UGA_BossPhase3::BindHP()
{
    if (HPChangeHandle.IsValid()) return;

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        HPChangeHandle =
            ASC->GetGameplayAttributeValueChangeDelegate(UMonsterAttributeSet::GetHealthAttribute())
            .AddUObject(this, &UGA_BossPhase3::OnHPChanged);
    }
}

void UGA_BossPhase3::UnbindHP()
{
    if (!HPChangeHandle.IsValid()) return;

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        ASC->GetGameplayAttributeValueChangeDelegate(UMonsterAttributeSet::GetHealthAttribute())
            .Remove(HPChangeHandle);
    }
    HPChangeHandle.Reset();
}

void UGA_BossPhase3::OnHPChanged(const FOnAttributeChangeData& Data)
{
    if (Data.NewValue <= 0.f)
    {
        EnterDeathCleanupFromP3(true);

        K2_EndAbility();
    }
}


void UGA_BossPhase3::EnterDeathCleanupFromP3(bool bTriggerDeathGA /*=true*/)
{
    bDisableWeakpointSpawns = true;   // ���� ���� ���� ����
    ForceKillPhaseMinions();          // ���� �̴Ͼ�� ����

    // Ÿ�̸�/�̵�/��Ŀ�� ����
    if (ACharacter* Boss = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
    {
        FTimerManager& TM = Boss->GetWorldTimerManager();
        TM.ClearTimer(RockTimer);
        TM.ClearTimer(AttackTimer);
        TM.ClearTimer(MinionTimer);

        if (AAIController* AI = Cast<AAIController>(Boss->GetController()))
        {
            if (MoveFinishedHandle.IsValid())
            {
                if (UPathFollowingComponent* PFC = AI->GetPathFollowingComponent())
                {
                    PFC->OnRequestFinished.Remove(MoveFinishedHandle);
                }
                MoveFinishedHandle.Reset();
            }
            AI->StopMovement();
            AI->ClearFocus(EAIFocusPriority::Gameplay);
        }

        // ���� ���� ��Ÿ�� ����(�浹 ����)
        if (UAnimInstance* Anim = Boss->GetMesh() ? Boss->GetMesh()->GetAnimInstance() : nullptr)
        {
            Anim->StopAllMontages(0.1f);
        }
    }

    RemoveInvuln();   // ���� ����
    UnbindHP();       // HP ��������Ʈ ����

    // (����) HitReact �� ���صǴ� GA ���, Death GA�� �����
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        // Death GA �±״� ������Ʈ�� �°� ��ü
        FGameplayTagContainer Ignore;
        // Ignore.AddTag(MonsterTags::Ability_Death);

        ASC->CancelAbilities(/*WithTags=*/nullptr, /*WithoutTags=*/Ignore.Num() ? &Ignore : nullptr);
    }

    // (����) Death GA Ʈ���� (������Ʈ ��Ģ�� �°� �̺�Ʈ/GE/�±׷� ȣ��)
    if (bTriggerDeathGA)
    {
        if (AActor* Boss = GetAvatarActorFromActorInfo())
        {
            // ��: �̺�Ʈ ���
            // FGameplayEventData Evt; Evt.EventTag = MonsterTags::Event_Death;
            // Evt.Instigator = Boss;
            // UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Boss, MonsterTags::Event_Death, Evt);
        }
    }
}

void UGA_BossPhase3::ApplyInvuln()
{
    if (InvulnHandle.IsValid() || !GE_BossInvuln_P3) return;

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
        FGameplayEffectSpecHandle    Spec = ASC->MakeOutgoingSpec(GE_BossInvuln_P3, 1.f, Ctx);
        if (Spec.IsValid())
        {
            InvulnHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
        }
    }
}

void UGA_BossPhase3::RemoveInvuln()
{
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        if (InvulnHandle.IsValid())
        {
            ASC->RemoveActiveGameplayEffect(InvulnHandle);
            InvulnHandle.Invalidate();
        }
    }
}

void UGA_BossPhase3::OnMinionDied(FGameplayEventData Payload)
{
    if (bDisableWeakpointSpawns)
        return;

    AActor* Boss = GetAvatarActorFromActorInfo();
    UWorld* World = Boss ? Boss->GetWorld() : nullptr;
    if (!World || !WeakPointClass) return;

    const FVector Center = Boss->GetActorLocation();

    // �Ķ����
    const float Rmin = 350.f;
    const float Rmax = 900.f;
    const int32 MaxTries = 24;
    const float PlaceCheckRadius = 80.f;     // �ڸ� ������� �˻� �ݰ�
    const float PlacementLift = 40.f;        // �ٴڰ��� ��ħ ���� ȸ�ǿ� ����Ʈ
    const ECollisionChannel GroundChannel = ECC_Visibility;

    // �ٴ�/�ڸ� üũ �� ������ ���
    FCollisionQueryParams QP(SCENE_QUERY_STAT(P3_SpawnWeakPoint), false, Boss);
    QP.AddIgnoredActor(Boss);

    if (const AActor* DeadMinion =
        Payload.Target.Get() ? Payload.Target.Get() :
        (Payload.Instigator.Get() ? Payload.Instigator.Get() :
            Cast<AActor>(Payload.OptionalObject)))
    {
        QP.AddIgnoredActor(DeadMinion);
    }

    // "�ڸ� �������"�� �ٴ�(WorldStatic)�� �����ϰ�,
    // Pawn/WorldDynamic/PhysicsBody � ������� ������ �˻�
    FCollisionObjectQueryParams ObjParams;
    ObjParams.AddObjectTypesToQuery(ECC_Pawn);
    ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);
    ObjParams.AddObjectTypesToQuery(ECC_PhysicsBody);

    // 1) XY ���ø� �� �ٴ����� Z�� ���� �� �ٸ� ���Ϳ� ��ġ�� �ʴ��� �˻�
    FVector SpawnLoc;
    bool bFound = false;

    for (int32 i = 0; i < MaxTries; ++i)
    {
        const float r = FMath::Sqrt(FMath::FRandRange(Rmin * Rmin, Rmax * Rmax));
        const float th = FMath::FRandRange(0.f, 2.f * PI);
        const FVector CandidateXY = Center + FVector(r * FMath::Cos(th), r * FMath::Sin(th), 0.f);

        // Z�� �ٴ� ����
        FVector GroundLoc;
        if (!ProjectToGround_NoTilt(World, CandidateXY, GroundLoc, 2000.f, 6000.f, 2.f, GroundChannel, &QP))
            continue;

        // �ٴ��� �����ϰ� "�ٸ� ����"�� ��ġ�� �ʴ��� �˻�
        const FCollisionShape Sphere = FCollisionShape::MakeSphere(PlaceCheckRadius);
        const bool bOverlapsOthers = World->OverlapAnyTestByObjectType(
            GroundLoc + FVector(0, 0, PlacementLift), // ��¦ ��� �˻�
            FQuat::Identity,
            ObjParams,
            Sphere,
            QP
        );

        if (!bOverlapsOthers)
        {
            SpawnLoc = GroundLoc;
            bFound = true;
            break;
        }
    }

    // 2) ���� ��: ���� ��ġ ���� �� XY�� �����ϵ�, �ݵ�� �ٴ� �������� Z Ȯ��
    if (!bFound)
    {
        const FVector FallbackXY = Center + FVector(200.f, 0.f, 0.f);
        if (!ProjectToGround_NoTilt(World, FallbackXY, SpawnLoc, 2000.f, 6000.f, 2.f, GroundChannel, &QP))
        {
            // ������ ������: Z�� �������� ���� ���缭 �ڱ�
            SpawnLoc = FVector(FallbackXY.X, FallbackXY.Y, Center.Z - 50.f);
        }
    }

    // 3) ƿ�� ����: ��ġ/�� 0, Yaw�� ������ ����(��ġ ������ 0.f)
    const float Yaw = Boss->GetActorRotation().Yaw;
    const FRotator SpawnRot(0.f, Yaw, 0.f);
    const FTransform SpawnTM(SpawnRot, SpawnLoc);

    // 4) Defer + NoCollision �� Finish �� ��ġ/ȸ�� �ڷ���Ʈ Ȯ�� �� �浹 ��Ȱ��
    AActor* Spawned = World->SpawnActorDeferred<AActor>(
        WeakPointClass,
        SpawnTM,
        Boss,
        nullptr,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn
    );
    if (!Spawned) return;

    if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Spawned->GetRootComponent()))
    {
        RootPrim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    if (AWeakPointActor* WP = Cast<AWeakPointActor>(Spawned))
    {
        if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
        {
            WP->InitializeWeakPoint(Boss, WeakPointDamageToBoss);
        }
    }

    UGameplayStatics::FinishSpawningActor(Spawned, SpawnTM);

    // ���� ���� ��ġ/ȸ�� �� �� �� Ȯ��(���� �ڷ���Ʈ)
    Spawned->SetActorLocation(SpawnLoc, /*bSweep=*/false, nullptr, ETeleportType::TeleportPhysics);
    Spawned->SetActorRotation(SpawnRot, ETeleportType::TeleportPhysics);

    // �浹 ��Ȱ�� (������Ʈ ��Ģ�� ���� ����)
    if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Spawned->GetRootComponent()))
    {
        RootPrim->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        // �ʿ� �� �������� ����:
        // RootPrim->SetCollisionProfileName(TEXT("WorldDynamic"));
    }
}

//void UGA_BossPhase3::OnMinionDied(FGameplayEventData Payload)
//{
//    AActor* Boss = GetAvatarActorFromActorInfo();
//    UWorld* World = Boss ? Boss->GetWorld() : nullptr;
//    if (!World || !WeakPointClass) return;
//
//    // 1) ���� �̴Ͼ� ���� ���� (Target �� Instigator �� OptionalObject ��)
//    AActor* DeadMinion = const_cast<AActor*>(
//        Payload.Target.Get() ? Payload.Target.Get() :
//        (Payload.Instigator.Get() ? Payload.Instigator.Get() :
//            Cast<AActor>(Payload.OptionalObject))
//        );
//
//    if (!DeadMinion) DeadMinion = Boss; // ������: ������ ���� �ֺ�
//
//    // 2) �Ķ����(����)
//    const float Rmin = 350.f;
//    const float Rmax = 900.f;
//    const int32 MaxTries = 24;
//    const float GroundTraceUp = 1200.f;
//    const float GroundTraceDown = 3000.f;
//    const float PlaceCheckRadius = 80.f;   // ������ �뷫 �ݰ�
//    const ECollisionChannel GroundChannel = ECC_Visibility;
//    const ECollisionChannel PlaceCheckChannel = ECC_Pawn; // �ʿ�� Ŀ���� ä�η� ��ü
//
//    // �浹 ���� ���
//    FCollisionQueryParams QP(SCENE_QUERY_STAT(P3_SpawnWeakPoint), false, Boss);
//    QP.AddIgnoredActor(Boss);
//    QP.AddIgnoredActor(DeadMinion);
//
//    // 3) �ĺ� ���ø� �� �ٴ� Ʈ���̽� �� �ڸ� ������� �� ����
//    auto TryFindSpot = [&](FVector& OutLoc, FRotator& OutRot)->bool
//        {
//            for (int32 i = 0; i < MaxTries; ++i)
//            {
//                const float r = FMath::Sqrt(FMath::FRandRange(Rmin * Rmin, Rmax * Rmax));
//                const float th = FMath::FRandRange(0.f, 2.f * PI);
//                const FVector XY = DeadMinion->GetActorLocation() + FVector(r * FMath::Cos(th), r * FMath::Sin(th), 0.f);
//
//                // �ٴ� ã��
//                FHitResult Hit;
//                const FVector Start = XY + FVector(0, 0, GroundTraceUp);
//                const FVector End = XY - FVector(0, 0, GroundTraceDown);
//                if (!World->LineTraceSingleByChannel(Hit, Start, End, GroundChannel, QP)) continue;
//
//                // ��¦ ���� ��� ����
//                const FVector Loc = Hit.ImpactPoint + Hit.Normal * 2.f;
//                const FRotator Rot = FRotationMatrix::MakeFromZ(Hit.Normal).Rotator();
//
//                // �ڸ� �������(�� ����)
//                const FCollisionShape Sphere = FCollisionShape::MakeSphere(PlaceCheckRadius);
//                FHitResult Block;
//                const FVector P = Loc + FVector(0, 0, 50.f); // ��¦ ������ ���� ����
//                const bool bBlocked = World->SweepSingleByChannel(Block, P, P, FQuat::Identity, PlaceCheckChannel, Sphere, QP);
//                if (!bBlocked) { OutLoc = Loc; OutRot = Rot; return true; }
//            }
//            return false;
//        };
//
//    FVector SpawnLoc;  FRotator SpawnRot;
//    if (!TryFindSpot(SpawnLoc, SpawnRot))
//    {
//        // 4) ������ ������: �̴Ͼ� ��ġ ���� ��
//        SpawnLoc = DeadMinion->GetActorLocation() + FVector(200.f, 0.f, -30.f);
//        SpawnRot = FRotator::ZeroRotator;
//    }
//
//    // 5) ���� & �ʱ�ȭ
//    if (AActor* Spawned = World->SpawnActor<AActor>(WeakPointClass, FTransform(SpawnRot, SpawnLoc)))
//    {
//        if (AWeakPointActor* WP = Cast<AWeakPointActor>(Spawned))
//        {
//            if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
//            {
//                WP->InitializeWeakPoint(Boss, WeakPointDamageToBoss);
//            }
//        }
//    }
//}

AActor* UGA_BossPhase3::GetTargetFromBlackboard() const
{
    ACharacter* Boss = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Boss) return nullptr;

    AAIController* AI = Cast<AAIController>(Boss->GetController());
    if (!AI) return nullptr;

    UBlackboardComponent* BB = AI->GetBlackboardComponent();
    if (!BB || BB_TargetActorKeyName.IsNone()) return nullptr;

    return Cast<AActor>(BB->GetValueAsObject(BB_TargetActorKeyName));
}

void UGA_BossPhase3::StartMoveToTargetOrAttack()
{
    ACharacter* Boss = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Boss) return;

    AActor* Target = GetTargetFromBlackboard();
    if (!Target)
    {
        // Ÿ���� ������ ��� �� ��õ�
        const float Delay = FMath::FRandRange(0.5f, 1.0f);
        Boss->GetWorldTimerManager().SetTimer(AttackTimer, this, &UGA_BossPhase3::Tick_Attack, Delay, false);
        return;
    }

    const float DistXY = FVector::Dist2D(Boss->GetActorLocation(), Target->GetActorLocation());
    if (DistXY <= ApproachAcceptanceRadius)
    {
        // �̹� ���� ���� �� ��ٷ� ����
        StartRandomAttackMontageOrReschedule();
        return;
    }

    AAIController* AI = Cast<AAIController>(Boss->GetController());
    if (!AI)
    {
        // ��Ʈ�ѷ� ������ �ٷ� ����(�ּ������� ���� ����)
        StartRandomAttackMontageOrReschedule();
        return;
    }

    // ���� ��������Ʈ �ߺ� ����
    if (MoveFinishedHandle.IsValid())
    {
        AI->GetPathFollowingComponent()->OnRequestFinished.Remove(MoveFinishedHandle);
        MoveFinishedHandle.Reset();
    }

    FAIMoveRequest Req;
    Req.SetGoalActor(Target);
    Req.SetAcceptanceRadius(ApproachAcceptanceRadius);
    Req.SetUsePathfinding(true);

    bMovingToAttack = true;

    FPathFollowingRequestResult R = AI->MoveTo(Req);
    CurrentMoveId = R.MoveId;

    // ����� �ٷ� �������� ��� ó��
    if (R.Code == EPathFollowingRequestResult::AlreadyAtGoal)
    {
        bMovingToAttack = false;
        StartRandomAttackMontageOrReschedule();
        return;
    }
    if (R.Code == EPathFollowingRequestResult::Failed)
    {
        bMovingToAttack = false;
        // �̵� ���� �� ª�� ���� ��õ�
        const float Delay = FMath::FRandRange(0.6f, 1.2f);
        Boss->GetWorldTimerManager().SetTimer(AttackTimer, this, &UGA_BossPhase3::Tick_Attack, Delay, false);
        return;
    }

    // ���������� �̵� ���� �� �Ϸ� �ݹ� ���ε�
    if (UPathFollowingComponent* PFC = AI->GetPathFollowingComponent())
    {
        MoveFinishedHandle = PFC->OnRequestFinished.AddUObject(
            this, &UGA_BossPhase3::OnMoveFinished);
    }
}


void UGA_BossPhase3::OnMoveFinished(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    bMovingToAttack = false;

    ACharacter* Boss = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    AAIController* AI = Boss ? Cast<AAIController>(Boss->GetController()) : nullptr;
    if (AI && MoveFinishedHandle.IsValid())
    {
        AI->GetPathFollowingComponent()->OnRequestFinished.Remove(MoveFinishedHandle);
        MoveFinishedHandle.Reset();
    }

    // ���� ��û�� MoveTo�� �ƴϸ� ����(�ٸ� �ý��۰� �浹 ����)
    if (RequestID.IsValid() && CurrentMoveId.IsValid() && RequestID != CurrentMoveId)
    {
        return;
    }

    // ���������� ����, ���и� ��õ�
    if (Result.IsSuccess())
    {
        StartRandomAttackMontageOrReschedule();
    }
    else
    {
        if (Boss)
        {
            const float Delay = FMath::FRandRange(0.5f, 1.0f);
            Boss->GetWorldTimerManager().SetTimer(AttackTimer, this, &UGA_BossPhase3::Tick_Attack, Delay, false);
        }
    }
}

void UGA_BossPhase3::StartRandomAttackMontageOrReschedule()
{
    ACharacter* Boss = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Boss) return;

    // ����� ��Ÿ�� ����
    TArray<UAnimMontage*> Candidates;
    for (UAnimMontage* M : AttackMontages)
    {
        if (M) Candidates.Add(M);
    }

    if (Candidates.Num() == 0)
    {
        // ��Ÿ�� ������ ���� �õ� ����
        const float Delay = FMath::FRandRange(AttackIntervalMin, AttackIntervalMax);
        Boss->GetWorldTimerManager().SetTimer(AttackTimer, this, &UGA_BossPhase3::Tick_Attack, Delay, false);
        return;
    }

    UAnimMontage* Pick = Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
    bAttackPlaying = true;

    if (UAbilityTask_PlayMontageAndWait* Task =
        UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Pick, 1.f, NAME_None, false))
    {
        Task->OnCompleted.AddDynamic(this, &UGA_BossPhase3::OnSmashMontageFinished);
        Task->OnBlendOut.AddDynamic(this, &UGA_BossPhase3::OnSmashMontageFinished);
        Task->OnInterrupted.AddDynamic(this, &UGA_BossPhase3::OnSmashMontageFinished);
        Task->OnCancelled.AddDynamic(this, &UGA_BossPhase3::OnSmashMontageFinished);
        Task->ReadyForActivation();
    }
    else
    {
        OnSmashMontageFinished();
    }
}


void UGA_BossPhase3::ForceKillPhaseMinions()
{
    // ���������� ����
    AActor* Boss = GetAvatarActorFromActorInfo();
    if (!Boss || !Boss->HasAuthority())
    {
        return;
    }

    if (!GE_ForceKillMinion)
    {
        UE_LOG(LogTemp, Warning, TEXT("[P3] GE_ForceKillMinion is not set."));
        // �׷��� �迭�� �����ϰ� ����
        PhaseMinions.Reset();
        return;
    }

    // �ڿ������� �����ϰ� ����
    for (int32 i = PhaseMinions.Num() - 1; i >= 0; --i)
    {
        ACharacter* Minion = PhaseMinions[i].Get();
        if (!Minion)
        {
            PhaseMinions.RemoveAt(i);
            continue;
        }

        UAbilitySystemComponent* MinionASC =
            UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Minion);
        if (!MinionASC)
        {
            PhaseMinions.RemoveAt(i);
            continue;
        }

        // �̹� �׾��ų�(HP<=0) Death ó�� ���̸� ��ŵ
        const float CurHP = MinionASC->GetNumericAttribute(UMonsterAttributeSet::GetHealthAttribute());
        if (CurHP <= 0.f)
        {
            PhaseMinions.RemoveAt(i);
            continue;
        }

        // Health�� 0���� ����� ���� GE ����
        {
            FGameplayEffectContextHandle Ctx = MinionASC->MakeEffectContext();
            Ctx.AddInstigator(Boss, Boss); // ��ó: ����

            FGameplayEffectSpecHandle Spec = MinionASC->MakeOutgoingSpec(GE_ForceKillMinion, 1.f, Ctx);
            if (Spec.IsValid())
            {
                MinionASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
            }
        }

        // ��Ͽ��� ���� (�ߺ� ���� ����)
        PhaseMinions.RemoveAt(i);
    }

    // �����ϰ� ����α�
    PhaseMinions.Shrink();
}