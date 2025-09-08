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
    if (AActor* Boss = GetAvatarActorFromActorInfo())
    {
        FTimerManager& TM = Boss->GetWorldTimerManager();
        TM.ClearTimer(RockTimer);
        TM.ClearTimer(AttackTimer);
        TM.ClearTimer(MinionTimer);
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

    // ����� ��Ÿ�� ����
    TArray<UAnimMontage*> Candidates;
    for (UAnimMontage* M : AttackMontages)
    {
        if (M) Candidates.Add(M);
    }
    if (Candidates.Num() == 0)
    {
        // ��Ÿ�ְ� ������ ���� üũ�� ����
        const float Delay = FMath::FRandRange(AttackIntervalMin, AttackIntervalMax);
        Boss->GetWorldTimerManager().SetTimer(AttackTimer, this, &UGA_BossPhase3::Tick_Attack, Delay, /*bLoop*/false);
        return;
    }

    UAnimMontage* Pick = Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
    bAttackPlaying = true;

    if (UAbilityTask_PlayMontageAndWait* Task =
        UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Pick, 1.f, NAME_None, /*bStopWhenAbilityEnds*/false))
    {
        Task->OnCompleted.AddDynamic(this, &UGA_BossPhase3::OnSmashMontageFinished);
        Task->OnBlendOut.AddDynamic(this, &UGA_BossPhase3::OnSmashMontageFinished);
        Task->OnInterrupted.AddDynamic(this, &UGA_BossPhase3::OnSmashMontageFinished);
        Task->OnCancelled.AddDynamic(this, &UGA_BossPhase3::OnSmashMontageFinished);
        Task->ReadyForActivation();
    }
    else
    {
        // ���� �� �ٷ� �÷��� ���� �� ����
        OnSmashMontageFinished();
    }
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
        K2_EndAbility();
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
    if (AActor* Boss = GetAvatarActorFromActorInfo())
    {
        UWorld* World = Boss->GetWorld();
        if (!World || !WeakPointClass) return;

        ACharacter* Player = UGameplayStatics::GetPlayerCharacter(World, 0);
        if (!Player) return;

        const float MinForwardDist = 600.f;   // �÷��̾� ���� �ּ� ���� �Ÿ�
        const float MaxForwardDist = 900.f;   // �ִ� ���� �Ÿ�
        const float LateralJitter = 250.f;   // �¿� ��鸲 ��
        const float PlaceCheckRadius = 80.f;    // �ڸ� ������� �˻��� �� �ݰ�
        const float GroundTraceUp = 1200.f;  // ������ ������� ����
        const float GroundTraceDown = 3000.f;  // �ٴ� ã�� ���� ����
        const int32 MaxTries = 18;      // �ĺ� �õ� Ƚ��
        const float YawStepDeg = 20.f;    // ���� ���� ��/�� ���� ����

        const FVector PLoc = Player->GetActorLocation();
        const FRotator PRot(0.f, Player->GetActorRotation().Yaw, 0.f);
        const FVector Fwd = PRot.Vector();
        const FVector Right = FRotationMatrix(PRot).GetUnitAxis(EAxis::Y);

        // �浹 ���� ���
        FCollisionQueryParams QP(SCENE_QUERY_STAT(P3_SpawnWeakPoint), false, Boss);
        QP.AddIgnoredActor(Boss);
        QP.AddIgnoredActor(Player);

        // �ĺ� ���� & �˻�
        auto FindGround = [&](const FVector& XY, FVector& OutLoc, FRotator& OutRot)->bool
            {
                // �ٴ� ã��
                const FVector Start = XY + FVector(0, 0, GroundTraceUp);
                const FVector End = XY - FVector(0, 0, GroundTraceDown);
                FHitResult Hit;
                if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QP))
                    return false;

                // ���� ����
                OutLoc = Hit.ImpactPoint + Hit.Normal * 2.f; // ��¦ ���
                OutRot = FRotationMatrix::MakeFromZ(Hit.Normal).Rotator();

                // �ڸ� ������� �� ���� �˻�
                FHitResult BlockHit;
                const FCollisionShape Sphere = FCollisionShape::MakeSphere(PlaceCheckRadius);
                bool bBlocked = World->SweepSingleByChannel(
                    BlockHit,
                    OutLoc + FVector(0, 0, 50.f),         // ��¦ ������
                    OutLoc + FVector(0, 0, 50.f),         // ���ڸ� ����
                    FQuat::Identity,
                    ECC_Pawn,                            // Ȥ�� Ŀ���� ä��
                    Sphere,
                    QP);

                return !bBlocked;
            };

        FVector ChosenLoc;
        FRotator ChosenRot;
        bool bFound = false;

        // 0��°: ���� + �¿� ���� ��鸲
        {
            const float Dist = FMath::FRandRange(MinForwardDist, MaxForwardDist);
            const float Side = FMath::FRandRange(-LateralJitter, LateralJitter);
            const FVector XY = PLoc + Fwd * Dist + Right * Side;
            bFound = FindGround(XY, ChosenLoc, ChosenRot);
        }

        // �� ã������ ������ �������� ��ä�� Ž��
        for (int32 Try = 0; !bFound && Try < MaxTries; ++Try)
        {
            const float Sign = (Try % 2 == 0) ? 1.f : -1.f;                 // ��/�� ������
            const float Steps = (Try + 1) * 0.5f;                               // 0.5,1.0,1.5...
            const float Yaw = Sign * Steps * YawStepDeg;

            const FVector Dir = FRotationMatrix(FRotator(0.f, Yaw, 0.f)).TransformVector(Fwd);
            const float Dist = FMath::FRandRange(MinForwardDist, MaxForwardDist);
            const float Side = FMath::FRandRange(-LateralJitter, LateralJitter);

            const FVector XY = PLoc + Dir * Dist + Right * Side;
            bFound = FindGround(XY, ChosenLoc, ChosenRot);
        }

        // ������ ������: �� ã������ �÷��̾� �ڸ� ���� ��
        if (!bFound)
        {
            FVector DummyLoc; FRotator DummyRot;
            FindGround(PLoc + Fwd * MinForwardDist, DummyLoc, DummyRot);
            ChosenLoc = DummyLoc; ChosenRot = DummyRot;
        }

        // ����!
        FTransform T(ChosenRot, ChosenLoc);
        if (AActor* Spawned = World->SpawnActor<AActor>(WeakPointClass, T))
        {
            if (AWeakPointActor* WP = Cast<AWeakPointActor>(Spawned))
            {
                if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
                {
                    WP->InitializeWeakPoint(Boss, WeakPointDamageToBoss);
                }
            }
        }
    }
}