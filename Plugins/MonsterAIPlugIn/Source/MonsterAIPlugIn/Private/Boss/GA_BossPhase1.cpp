
#include "Boss/GA_BossPhase1.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "TimerManager.h"
#include "Sound/SoundBase.h"

#include "MonsterTags.h"
#include "MonsterAttributeSet.h"
#include "Monster/MonsterCharacter.h"
#include "Boss/BossCharacter.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Data/staticName.h"
#include "Boss/FallingRockActor.h"
#include "Components/CapsuleComponent.h"


static bool FindSafeSpawnLocation(
    UWorld* World,
    const FVector& DesiredXY,              // ���ϴ� XY �߽� (Z�� ����)
    float CapsuleRadius, float CapsuleHalfHeight,
    FVector& OutLoc,
    float NavSearchExtent = 300.f,         // �׺� ���� extents
    float GroundTraceUp = 1000.f,          // ���� Ž�� ����
    float GroundTraceDown = 2000.f,        // �Ʒ��� Ž�� ����
    int32 MaxAngleSamples = 16,            // �� �ݰ濡�� �õ��� ���� ����
    int32 RadiusSteps = 6,                 // �ݰ� ���� �ܰ� ��
    float InitialProbe = 0.f,              // ���� ���� �ݰ�
    float Step = 60.f                      // ���� �ݰ� ������
) {
    if (!World) return false;

    FVector Base = DesiredXY;

    // 1) �׺� ����(������ ���)
    if (UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(World))
    {
        FNavLocation Out;
        if (Nav->ProjectPointToNavigation(DesiredXY, Out, FVector(NavSearchExtent)))
        {
            Base = Out.Location;
        }
    }

    auto GroundSnap = [&](const FVector& InXY, FVector& Out) -> bool
        {
            FVector Start = InXY + FVector(0, 0, GroundTraceUp);
            FVector End = InXY - FVector(0, 0, GroundTraceDown);
            FHitResult Hit;
            if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
            {
                Out = Hit.ImpactPoint;
                Out.Z += CapsuleHalfHeight + 2.f; // �ٴڿ��� ��¦ ���
                return true;
            }
            // Ʈ���̽� ���� ��, ���� Z ����(�ּ����� ���)
            Out = FVector(InXY.X, InXY.Y, DesiredXY.Z + CapsuleHalfHeight + 2.f);
            return false;
        };

    auto IsFree = [&](const FVector& L) -> bool
        {
            FCollisionShape Shape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);
            // Pawn ä�� �������� ��ħ �˻�(������Ʈ�� �°� ä�� ����)
            return !World->OverlapBlockingTestByChannel(
                L, FQuat::Identity, ECC_Pawn, Shape,
                FCollisionQueryParams(SCENE_QUERY_STAT(FindSafeSpawn_Overlap), false));
        };

    // 2) �⺻ ��ġ Z ���߰� �˻�
    FVector TryLoc;
    GroundSnap(Base, TryLoc);
    if (IsFree(TryLoc)) { OutLoc = TryLoc; return true; }

    // 3) �������� �ݰ��� �ø��� ���ڸ� Ž��
    for (int32 r = 0; r < RadiusSteps; ++r)
    {
        const float R = InitialProbe + Step * r;
        for (int32 i = 0; i < MaxAngleSamples; ++i)
        {
            const float Theta = (2.f * PI) * (float(i) / MaxAngleSamples);
            const FVector XY = Base + FVector(R * FMath::Cos(Theta), R * FMath::Sin(Theta), 0.f);

            GroundSnap(XY, TryLoc);
            if (IsFree(TryLoc)) { OutLoc = TryLoc; return true; }
        }
    }

    return false; // ������ ����
}

static FVector RandomPointInAnnulus2D(const FVector& Center, float Rmin, float Rmax)
{
    const float R = FMath::Sqrt(FMath::FRandRange(Rmin * Rmin, Rmax * Rmax));
    const float Theta = FMath::FRandRange(0.f, 2 * PI);
    return Center + FVector(R * FMath::Cos(Theta), R * FMath::Sin(Theta), 0.f);
}

UGA_BossPhase1::UGA_BossPhase1()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    FGameplayTagContainer AssetTags;
    AssetTags.AddTag(MonsterTags::Ability_Boss_PhaseStart);
    SetAssetTags(AssetTags);

    ActivationBlockedTags.AddTag(MonsterTags::State_Dying);
    ActivationBlockedTags.AddTag(MonsterTags::State_Dead);
    BlockAbilitiesWithTag.AddTag(MonsterTags::Ability_HitReact);
}

void UGA_BossPhase1::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, Info, ActivationInfo, TriggerEventData);

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    AActor* Boss = GetAvatarActorFromActorInfo();
    if (!ASC || !Boss)
    {
        EndAbility(Handle, Info, ActivationInfo, true, false);
        return;
    }

    const float CurHP = ASC->GetNumericAttribute(UMonsterAttributeSet::GetHealthAttribute());
    const float MaxHP = ASC->GetNumericAttribute(UMonsterAttributeSet::GetMaxHealthAttribute());
    const float Ratio = (MaxHP > 0.f) ? (CurHP / MaxHP) : 0.f;

    if (Ratio <= StartHpRatioThreshold && !bPhaseStarted)
    {
        StartPhase();
        return;
    }

    // ���� �Ӱ� �� �� HP ��ȭ ���� ���ε�
    BindHPThresholdWatch();
}

void UGA_BossPhase1::BindHPThresholdWatch()
{
    if (HPChangeHandle.IsValid()) return;

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        HPChangeHandle = ASC->GetGameplayAttributeValueChangeDelegate(
            UMonsterAttributeSet::GetHealthAttribute()
        ).AddUObject(this, &UGA_BossPhase1::OnHPChangedNative);
    }
}

void UGA_BossPhase1::UnbindHPThresholdWatch()
{
    if (!HPChangeHandle.IsValid()) return;

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        ASC->GetGameplayAttributeValueChangeDelegate(
            UMonsterAttributeSet::GetHealthAttribute()
        ).Remove(HPChangeHandle);
    }
    HPChangeHandle.Reset();
}

void UGA_BossPhase1::OnHPChangedNative(const FOnAttributeChangeData& Data)
{
    if (bPhaseStarted) { UnbindHPThresholdWatch(); return; }

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        const float Max = ASC->GetNumericAttribute(UMonsterAttributeSet::GetMaxHealthAttribute());
        if (Max <= 0.f) return;

        const float Ratio = Data.NewValue / Max;
        if (Ratio <= StartHpRatioThreshold)
        {
            UnbindHPThresholdWatch();
            StartPhase();
        }
    }
}

void UGA_BossPhase1::StartPhase()
{
    if (bPhaseStarted) return;
    bPhaseStarted = true;
    ////�ǰݸ�� X
    //if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    //{
    //    FGameplayTagContainer CancelTags;
    //    CancelTags.AddTag(MonsterTags::Ability_HitReact);
    //    ASC->CancelAbilities(&CancelTags);
    //}


    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    AActor* Boss = GetAvatarActorFromActorInfo();
    if (!ASC || !Boss)
    {
        K2_EndAbility();
        return;
    }

    Boss->GetWorldTimerManager().SetTimerForNextTick(this, &UGA_BossPhase1::BeginStartSequence);
    



    // A) ���� GE ���� (���� GCN �ڵ� ����)
    if (GE_BossInvuln)
    {
        FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
        Ctx.AddInstigator(Boss, Boss->GetInstigatorController());
        FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(GE_BossInvuln, 1.f, Ctx);
        InvulnHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
    }

    // B) ���� ����: ��Ÿ�� + ����
    //if (StartMontage)
    //{
    //    if (auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
    //        this, NAME_None, StartMontage, 1.f, NAME_None, false))
    //    {
    //        Task->ReadyForActivation();
    //    }
    //}
    if (StartSound)
    {
        if (ACharacter* Char = Cast<ACharacter>(Boss))
        {
            UGameplayStatics::SpawnSoundAttached(StartSound, Char->GetMesh());
        }
        else
        {
            UGameplayStatics::PlaySoundAtLocation(Boss, StartSound, Boss->GetActorLocation());
        }
    }

    // C) ��ȯ
    AliveMinionCount = SpawnMinions(Boss);

    // D) ��ȯ�� ��� �̺�Ʈ ��� (�̴Ͼ� Death GA���� Event.Minion.Died �۽�)
    if (UAbilityTask_WaitGameplayEvent* WaitEvt =
        UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, MonsterTags::Event_Minion_Died))
    {
        WaitEvt->EventReceived.AddDynamic(this, &UGA_BossPhase1::OnMinionDiedEvent);
        WaitEvt->ReadyForActivation();
    }

    if (ABossCharacter* BossCharacter = Cast<ABossCharacter>(GetAvatarActorFromActorInfo()))
    {
        BossCharacter->SetBossState_EBB((uint8)EBossState_BB::InPhase1);
    }


    // E) ĳ���� ���� ����
    //StartCastTick();
}

AActor* UGA_BossPhase1::GetPhaseTargetPlayer() const
{
    // �ʿ信 ���� �÷��̾� �ε���/Ÿ�� ��Ģ ����
    return UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
}

// ----- �̴Ͼ� BB ���� �õ� (���� �� true) -----
bool UGA_BossPhase1::TrySetupMinionBlackboard(AMonsterCharacter* MC, AActor* Player)
{
    if (!MC) return false;

    // ��Ʈ�ѷ� ����
    AAIController* AICon = Cast<AAIController>(MC->GetController());
    if (!AICon)
    {
        MC->SpawnDefaultController();
        AICon = Cast<AAIController>(MC->GetController());
        if (!AICon) return false; // ���� ��Ʈ�ѷ��� ���� �� ��õ�
    }

    UBlackboardComponent* BB = AICon->GetBlackboardComponent();
    if (!BB) return false; // BB ���� �ʱ�ȭ �� �� ��õ�

    // �� ����
    //BB->SetValueAsObject(KEY_TargetActor, Player);
    BB->SetValueAsEnum(KEY_MonsterState, DesiredMinionState);


    return true;
}

// ----- ��⿭�� �߰� -----
void UGA_BossPhase1::EnqueueMinionInit(AMonsterCharacter* MC)
{
    if (!MC) return;
    PendingInitMinions.AddUnique(MC);

    // Ÿ�̸Ӱ� ���� ���� ������ ����
    AActor* Boss = GetAvatarActorFromActorInfo();
    if (Boss && !Boss->GetWorldTimerManager().IsTimerActive(MinionInitTimerHandle))
    {
        Boss->GetWorldTimerManager().SetTimer(
            MinionInitTimerHandle, this, &UGA_BossPhase1::ProcessPendingMinionInits, 0.1f, true, 0.1f);
    }
}

// ----- ��⿭ ó�� -----
void UGA_BossPhase1::ProcessPendingMinionInits()
{
    AActor* Player = GetPhaseTargetPlayer();
    if (!Player)
    {
        // �÷��̾� ������ ���� ƽ�� �ٽ�
        return;
    }

    // �ڿ������� ����
    for (int32 i = PendingInitMinions.Num() - 1; i >= 0; --i)
    {
        TWeakObjectPtr<AMonsterCharacter> WeakMC = PendingInitMinions[i];
        AMonsterCharacter* MC = WeakMC.Get();
        if (!MC)
        {
            PendingInitMinions.RemoveAtSwap(i);
            continue;
        }

        if (TrySetupMinionBlackboard(MC, Player))
        {
            PendingInitMinions.RemoveAtSwap(i);
        }
    }

    // ��� ó�� ������ Ÿ�̸� ����
    if (PendingInitMinions.Num() == 0)
    {
        if (AActor* Boss = GetAvatarActorFromActorInfo())
        {
            Boss->GetWorldTimerManager().ClearTimer(MinionInitTimerHandle);
        }
    }
}


int32 UGA_BossPhase1::SpawnMinions(AActor* Boss)
{
    if (!Boss || !MinionClass) return 0;

    UWorld* World = Boss->GetWorld();
    if (!World) return 0;

    const int32 Count = FMath::Max(1, FMath::RandRange(SpawnCountMin, SpawnCountMax));
    const FVector Origin = Boss->GetActorLocation();
    const float StepDeg = 360.f / Count;

    UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(World);
    AActor* Player = GetPhaseTargetPlayer();

    float CapR = 42.f, CapH = 96.f;
    if (const ACharacter* CDO = Cast<ACharacter>(MinionClass->GetDefaultObject()))
    {
        if (const UCapsuleComponent* Cap = CDO->GetCapsuleComponent())
        {
            CapR = Cap->GetUnscaledCapsuleRadius();
            CapH = Cap->GetUnscaledCapsuleHalfHeight();
        }
    }

    // ���� �Ķ����: �浹 ���� ���
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    int32 SpawnedCount = 0;

    for (int32 i = 0; i < Count; ++i)
    {
        const float Deg = StepDeg * i;
        const FVector Dir = UKismetMathLibrary::GetForwardVector(FRotator(0.f, Deg, 0.f));
        const FVector DesiredXY = Origin + Dir * SpawnRadius; // Z�� ���߿� �ٴ� ��������

        // Nav�� �� �� ����(������)
        FVector NavBased = DesiredXY;
        if (Nav)
        {
            FNavLocation Out;
            if (Nav->ProjectPointToNavigation(DesiredXY, Out, FVector(300.f)))
            {
                NavBased = Out.Location;
            }
        }

        // ������ ��ġ ã�� �õ�
        FVector SafeLoc;
        const bool bFound = FindSafeSpawnLocation(
            World, NavBased, CapR, CapH, SafeLoc,
            /*NavSearchExtent=*/300.f,
            /*GroundTraceUp=*/1000.f,
            /*GroundTraceDown=*/2000.f,
            /*MaxAngleSamples=*/16,
            /*RadiusSteps=*/6,
            /*InitialProbe=*/0.f,
            /*Step=*/FMath::Max(60.f, CapR * 1.5f) // ĸ�� ũ�⿡ ����� ����
        );

        const FVector UseLoc = bFound ? SafeLoc
            : (NavBased + FVector(0, 0, CapH + 2.f)); // ���� �ÿ��� ����

        const FTransform T(FRotator::ZeroRotator, UseLoc);

        AActor* Spawned = World->SpawnActor<AActor>(MinionClass, T, Params);
        if (AMonsterCharacter* MC = Cast<AMonsterCharacter>(Spawned))
        {
            ++SpawnedCount;

            MC->SetOwnerBoss(GetAvatarActorFromActorInfo());

            if (!Player || !TrySetupMinionBlackboard(MC, Player))
            {
                EnqueueMinionInit(MC);
            }
        }
    }

    return SpawnedCount;
}

void UGA_BossPhase1::StartCastTick()
{
    AActor* Boss = GetAvatarActorFromActorInfo();
    if (!Boss || CastInterval <= 0.f) return;


    Boss->GetWorldTimerManager().SetTimer(
        CastTimerHandle,
        [this]() { DoCastOnce(); },
        CastInterval,
        true,
        CastInterval
    );
}

void UGA_BossPhase1::DoCastOnce()
{
    if (CastLoopMontage)
    {
        if (auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, NAME_None, CastLoopMontage, 1.f, NAME_None, false))
        {
            Task->ReadyForActivation();
        }

        if (!FallingRockClass) return;

        UWorld* World = GetWorld();
        AActor* Player = GetPhaseTargetPlayer();
        AActor* Boss = GetAvatarActorFromActorInfo();
        if (!World || !Player || !Boss) return;

        int32 numRocks = FMath::Clamp(FMath::RandRange(RocksPerCastMin, RocksPerCastMax), 1, 50);
        FVector playerLoc = Player->GetActorLocation();

        for (int32 i = 0; i < numRocks; ++i)
        {
            // �÷��̾� ���� ���� ������(����)
            FVector groundXY = RandomPointInAnnulus2D(playerLoc, PlayerAreaRadiusMin, PlayerAreaRadiusMax);
            // ������ �������� ���� ��ġ ����
            FVector spawnLoc = FVector(groundXY.X, groundXY.Y, playerLoc.Z + SpawnHeight);

            FTransform T(FRotator::ZeroRotator, spawnLoc);

            AFallingRockActor* rock = World->SpawnActorDeferred<AFallingRockActor>(
                FallingRockClass,
                T,
                GetAvatarActorFromActorInfo(),                            // Owner = ����
                Cast<APawn>(GetAvatarActorFromActorInfo()),               // Instigator = ����Pawn
                ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

            if (rock)
            {
                // ������ ���� Instigator ����
                rock->SetDamageInstigator(GetAvatarActorFromActorInfo());

                // ������ ����
                rock->DamageMagnitude = RockDamage;                       // Phase1�� �����ص� ��
                rock->ByCallerDamageTag = MonsterTags::Data_Drop_Damage;

                // ������Ÿ��
                if (RockLifeSeconds > 0.f) rock->SetLifeSpan(RockLifeSeconds);

                rock->FinishSpawning(T);
            }

            if (RockImpactSound && RockImpactSfxDelay > 0.f)
            {
                // ���� ��ġ ����(���� ����Ʈ���̽�)
                FVector s = spawnLoc;
                FVector e = s + FVector(0, 0, -10000.f);
                FHitResult hit;
                FCollisionQueryParams qp(SCENE_QUERY_STAT(Phase1_RockSFXGround), false, Boss);
                qp.AddIgnoredActor(Boss);
                bool bHit = World->LineTraceSingleByObjectType(
                    hit, s, e,
                    FCollisionObjectQueryParams(FCollisionObjectQueryParams::AllStaticObjects),
                    qp
                );

                const FVector impactLoc = bHit ? hit.ImpactPoint : FVector(groundXY.X, groundXY.Y, playerLoc.Z);

                // Ÿ�̸ӷ� n�� �� ���
                FTimerHandle th;
                Boss->GetWorldTimerManager().SetTimer(
                    th,
                    FTimerDelegate::CreateWeakLambda(this, [this, Boss, impactLoc]()
                        {
                            if (!IsValid(Boss)) return; // ���� ���� ��
                            if (RockImpactSound)
                            {
                                UGameplayStatics::PlaySoundAtLocation(Boss, RockImpactSound, impactLoc);
                            }
                        }),
                    RockImpactSfxDelay,
                    false
                );
                RockSfxTimerHandles.Add(th);
            }
        }
    }

    // TODO: ���⼭ ����ü/�������� �� ���� ĳ���� ���� ȣ��
}

void UGA_BossPhase1::OnMinionDiedEvent(FGameplayEventData Payload)
{
    if (AliveMinionCount > 0) --AliveMinionCount;

    if (AliveMinionCount == 0)
    {
        //������������ �� endAbility
        //if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
        //{
        //    if (InvulnHandle.IsValid())
        //    {
        //        ASC->RemoveActiveGameplayEffect(InvulnHandle); // ���� ����(���� GCN �ڵ� Off)
        //        InvulnHandle.Invalidate();
        //    }
        //}
        //K2_EndAbility();
        BeginEndSequence();
    }
}

void UGA_BossPhase1::EndAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    if (AActor* Boss = GetAvatarActorFromActorInfo())
    {
        Boss->GetWorldTimerManager().ClearTimer(CastTimerHandle);
        // ������� ����Ʈ ���� Ÿ�̸� ��� ����
        for (FTimerHandle& H : RockSfxTimerHandles)
        {
            Boss->GetWorldTimerManager().ClearTimer(H);
        }
        RockSfxTimerHandles.Empty();
    }

    if (AActor* Boss = GetAvatarActorFromActorInfo())
    {
        Boss->GetWorldTimerManager().ClearTimer(CastTimerHandle);
    }

    UnbindHPThresholdWatch();

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        if (InvulnHandle.IsValid())
        {
            ASC->RemoveActiveGameplayEffect(InvulnHandle);
            InvulnHandle.Invalidate();
        }
        ASC->RemoveGameplayCue(MonsterTags::GC_Boss_InvulnShield);
    }
    
    if (ABossCharacter* Boss = Cast<ABossCharacter>(GetAvatarActorFromActorInfo()))
    {
        // ĳ����/��ȯ ������ ���� �� ���� �������
        Boss->SetBossState_EBB((uint8)EBossState_BB::Phase1_Attack);
    }

    Super::EndAbility(Handle, Info, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}


void UGA_BossPhase1::BeginStartSequence()
{
    // 1) Hit GA ���
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (ASC)
    {
        FGameplayTagContainer CancelTags;
        CancelTags.AddTag(MonsterTags::Ability_HitReact);
        ASC->CancelAbilities(&CancelTags);
    }

    // 2) Ȥ�� ���� ��Ÿ�� ����(�׷� �Ǵ� ��ü)
    ACharacter* C = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (C && C->GetMesh())
    {
        UAnimInstance* Anim = C->GetMesh()->GetAnimInstance();
        if (Anim)
        {
            // ���� Ȯ��: ��ü ����(�ʿ� �� �׷츸 ������ �ٲټ���)
            Anim->StopAllMontages(0.10f);
            // �Ǵ�: Anim->Montage_StopGroupByName(0.10f, FName("DefaultGroup"));
        }
    }

    if (AActor* Boss = GetAvatarActorFromActorInfo())
    {
        Boss->GetWorldTimerManager().SetTimerForNextTick(this, &UGA_BossPhase1::PlayStartMontageThenStartCast);
    }
}

void UGA_BossPhase1::PlayStartMontageThenStartCast()
{
    if (!StartMontage)
    {
        StartCastTick();
        return;
    }

    if (auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, StartMontage, 1.f, NAME_None, /*bStopWhenAbilityEnds=*/false))
    {
        Task->OnCompleted.AddDynamic(this, &UGA_BossPhase1::StartCastTick);
        Task->OnBlendOut.AddDynamic(this, &UGA_BossPhase1::StartCastTick);
        Task->OnInterrupted.AddDynamic(this, &UGA_BossPhase1::StartCastTick);
        Task->OnCancelled.AddDynamic(this, &UGA_BossPhase1::StartCastTick);
        Task->ReadyForActivation();
    }
    else
    {
        // ������
        StartCastTick();
    }
}


void UGA_BossPhase1::BeginEndSequence()
{
    if (bEnding) return;
    bEnding = true;

    AActor* Boss = GetAvatarActorFromActorInfo();

    // 1) ĳ���� ���� �ߴ�
    if (Boss)
    {
        Boss->GetWorldTimerManager().ClearTimer(CastTimerHandle);
    }

    // 2) �ǰ�/��Ÿ GA�� ���� ���ϰ� HitReact �� ���(����)
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        FGameplayTagContainer CancelTags;
        CancelTags.AddTag(MonsterTags::Ability_HitReact);
        ASC->CancelAbilities(&CancelTags);
    }

    // 3) ���� ���� ��Ÿ�� ����(ĳ��Ʈ ���� ��Ÿ�� ��)
    if (ACharacter* C = Cast<ACharacter>(Boss))
    {
        if (UAnimInstance* Anim = C->GetMesh() ? C->GetMesh()->GetAnimInstance() : nullptr)
        {
            Anim->StopAllMontages(0.10f); // �ʿ��ϸ� �׷츸 ������ �ٲ㵵 OK
        }
    }

    // 4) End ��Ÿ�� ���(������ �ٷ� ����)
    PlayEndMontageAndFinish();
}

void UGA_BossPhase1::PlayEndMontageAndFinish()
{
    if (EndMontage)
    {
        if (auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, NAME_None, EndMontage, 1.f, NAME_None, /*bStopWhenAbilityEnds=*/false))
        {
            // � ���� ������ Ability ����
            Task->OnCompleted.AddDynamic(this, &UGA_BossPhase1::K2_EndAbility);
            Task->OnBlendOut.AddDynamic(this, &UGA_BossPhase1::K2_EndAbility);
            Task->OnInterrupted.AddDynamic(this, &UGA_BossPhase1::K2_EndAbility);
            Task->OnCancelled.AddDynamic(this, &UGA_BossPhase1::K2_EndAbility);
            Task->ReadyForActivation();
            return;
        }
    }

    // EndMontage�� ���ų� Task ���� ���� �� ������
    K2_EndAbility();
}