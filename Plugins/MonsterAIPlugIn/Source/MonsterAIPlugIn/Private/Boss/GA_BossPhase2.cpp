#include "Boss/GA_BossPhase2.h"
#include "AbilitySystemComponent.h"
#include "MonsterAttributeSet.h"
#include "MonsterTags.h"
#include "GameFramework/Actor.h"
#include "Boss/BossCharacter.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Data/staticName.h"

#include "Kismet/GameplayStatics.h"           
#include "Components/SkeletalMeshComponent.h" 
#include "Boss/WeakPointActor.h"
#include "NavigationSystem.h"
#include "Boss/ShockwaveActor.h"
static FVector RandomOnRing2D(const FVector& _Center, float _Radius)
{
    float Theta = FMath::FRandRange(0.f, 2.f * PI);
    return _Center + FVector(_Radius * FMath::Cos(Theta), _Radius * FMath::Sin(Theta), 0.f);
}

static bool ProjectToGround_NoTilt(
    UWorld* World,
    const FVector& XY,                   // X,Y�� �ǹ� ����
    FVector& OutGroundLoc,               // ���� ���� ��ġ(Z Ȯ��)
    float TraceUp = 1500.f,
    float TraceDown = 4000.f,
    float GroundOffset = 2.f,            // ������� ����
    ECollisionChannel GroundChannel = ECC_Visibility,
    const FCollisionQueryParams* InParams = nullptr
)
{
    if (!World) return false;

    FCollisionQueryParams Params = InParams ? *InParams : FCollisionQueryParams(SCENE_QUERY_STAT(P2_WeakGround), false);
    const FVector Start = FVector(XY.X, XY.Y, XY.Z + TraceUp);
    const FVector End = FVector(XY.X, XY.Y, XY.Z - TraceDown);

    FHitResult Hit;
    if (World->LineTraceSingleByChannel(Hit, Start, End, GroundChannel, Params))
    {
        OutGroundLoc = Hit.ImpactPoint + FVector(0, 0, GroundOffset); // Z�� ����
        return true;
    }
    return false;
}

UGA_BossPhase2::UGA_BossPhase2()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    // �±״� ������Ʈ ��Ģ�� �°� �����ϼ���
    FGameplayTagContainer AssetTags;
    AssetTags.AddTag(MonsterTags::Ability_Boss_PhaseStart);
    // ����: ���� �ĺ� �±׵� ������ �߰�
    // AssetTags.AddTag(MonsterTags::Ability_Boss_Phase2);
    SetAssetTags(AssetTags);

    ActivationBlockedTags.AddTag(MonsterTags::State_Dying);
    ActivationBlockedTags.AddTag(MonsterTags::State_Dead);
}

void UGA_BossPhase2::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, Info, ActivationInfo, TriggerEventData);

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (!ASC)
    {
        EndAbility(Handle, Info, ActivationInfo, true, false);
        return;
    }

    const float CurHP = ASC->GetNumericAttribute(UMonsterAttributeSet::GetHealthAttribute());
    const float MaxHP = ASC->GetNumericAttribute(UMonsterAttributeSet::GetMaxHealthAttribute());
    const float Ratio = (MaxHP > 0.f) ? (CurHP / MaxHP) : 0.f;
   
   
    if (Ratio <= EndHpRatioThreshold)
    {

        bShouldEndAfterCurrentSmash = true;
        // Ÿ�̸� �̰��� ���� ����
        if (AActor* Boss = GetAvatarActorFromActorInfo())
        {
            Boss->GetWorldTimerManager().ClearTimer(SmashTimerHandle);
        }
        // ���� ���Žð� ������ �ٷ� ����
        BeginEndSequence();
        return;
    }

    if (Ratio <= StartHpRatioThreshold && !bPhaseStarted)
    {
       
        StartPhase();
        return;
    }

    BindHPThresholdWatch();
}

void UGA_BossPhase2::BindHPThresholdWatch()
{
    if (HPChangeHandle.IsValid()) return;

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        HPChangeHandle = ASC->GetGameplayAttributeValueChangeDelegate(
            UMonsterAttributeSet::GetHealthAttribute()
        ).AddUObject(this, &UGA_BossPhase2::OnHPChangedNative);
    }
}

void UGA_BossPhase2::UnbindHPThresholdWatch()
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

void UGA_BossPhase2::OnHPChangedNative(const FOnAttributeChangeData& Data)
{
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        const float Max = ASC->GetNumericAttribute(UMonsterAttributeSet::GetMaxHealthAttribute());
        if (Max <= 0.f) return;

        const float Ratio = Data.NewValue / Max;

        // End ����: �׻� üũ
        if (Ratio <= EndHpRatioThreshold && !bShouldEndAfterCurrentSmash && !bEnding)
        {
            bShouldEndAfterCurrentSmash = true;

            if (AActor* Boss = GetAvatarActorFromActorInfo())
            {
                Boss->GetWorldTimerManager().ClearTimer(SmashTimerHandle); // �� ���Ž� ����
            }

            if (!bSmashInProgress)
            {
                BeginEndSequence(); // ���Ž� ���� �ƴϸ� �ٷ� ����
                return;
            }
            // ���Ž� ���̸� OnSmashMontageFinished���� ���� ����
        }

        // Start ����: ���� Phase ���� �� ���� ����
        if (!bPhaseStarted && Ratio <= StartHpRatioThreshold)
        {

            UnbindHPThresholdWatch();
            StartPhase();
        }
    }
}

void UGA_BossPhase2::StartPhase()
{
    if (bPhaseStarted) return;
    bPhaseStarted = true;

    if (ABossCharacter* BossCharacter = Cast<ABossCharacter>(GetAvatarActorFromActorInfo()))
    {
        BossCharacter->SetBossState_EBB((uint8)EBossState_BB::InPhase2);
    }

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        const float CurHP = ASC->GetNumericAttribute(UMonsterAttributeSet::GetHealthAttribute());
        const float MaxHP = ASC->GetNumericAttribute(UMonsterAttributeSet::GetMaxHealthAttribute());
        const float TargetHP = MaxHP * EndHpRatioThreshold;  // ��: 0.20 �� 20%

        const float Need = FMath::Max(0.f, CurHP - TargetHP);   // ���ݺ��� ��ǥ���� ��ƾ� �� �ѷ�
        const int32 Count = FMath::Max(1, WeakPointSpawnCount); // 0 ������ ����

        float perWeak = Need / Count;

        // ��Ȥ�� �𸣴ϱ� 1 ������ Ű���ڡ�
        if (Need > KINDA_SMALL_NUMBER)
        {
            perWeak += 1.f;
        }

        // ��û�Ͻ� ��� ���������� ����(Health�� Add�� ���̴� GE��� �������� ü���� ����)
        WeakPointDamageToBoss = -perWeak;

        UE_LOG(LogTemp, Log, TEXT("[Phase2] Cur=%.1f Max=%.1f Target=%.1f Need=%.1f Count=%d PerWeak=%.1f(neg)"),
            CurHP, MaxHP, TargetHP, Need, Count, WeakPointDamageToBoss);
    }

    BindHPThresholdWatch();

    ApplyInvuln();

    BeginStartSequence();
}

void UGA_BossPhase2::EndAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    if (AActor* Boss = GetAvatarActorFromActorInfo())
    {
        Boss->GetWorldTimerManager().ClearTimer(SmashTimerHandle);
    }
    bSmashInProgress = false;
    bStartingPhase2 = false;

    // ���� ���� ����/���Ž� ��Ÿ�ְ� �ִٸ� �����ϰ� ����
    if (ACharacter* C = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
    {
        if (UAnimInstance* Anim = C->GetMesh() ? C->GetMesh()->GetAnimInstance() : nullptr)
        {
            Anim->StopAllMontages(0.10f);
        }
    }


    UnbindHPThresholdWatch();

    if (ABossCharacter* Boss = Cast<ABossCharacter>(GetAvatarActorFromActorInfo()))
    {
        // ĳ����/��ȯ ������ ���� �� ���� �������
        Boss->SetBossState_EBB((uint8)EBossState_BB::InPhase3);
    }

    RemoveInvuln();

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        if (GA_BossPhase3Class) // UPROPERTY�� GA_BossPhase3Class�� �޾Ƶθ� ��
        {
            FGameplayAbilitySpec Spec(GA_BossPhase3Class, 1, INDEX_NONE, GetAvatarActorFromActorInfo());
            FGameplayAbilitySpecHandle NewHandle = ASC->GiveAbility(Spec);
            ASC->TryActivateAbility(NewHandle);
        }
    }

    

    Super::EndAbility(Handle, Info, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_BossPhase2::SmashTick()
{
    if (bShouldEndAfterCurrentSmash || bEnding)
    {
        // ���� ���Žø� �������� �ʰ� ��������
        BeginEndSequence();
        return;
    }


    if (bSmashInProgress) return; // ���� ���� ���� ���� ���̸� ��ŵ
    bSmashInProgress = true;

    ACharacter* C = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!C || !GroundSmashMontage) { bSmashInProgress = false; return; }

    // ����(��Ʈ������� �����ϸ� Launch ����)
    if (JumpZ > 0.f) C->LaunchCharacter(FVector(0, 0, JumpZ), false, true);

    // ��Ÿ��: Start��Loop(����) ����
    if (UAnimInstance* Anim = C->GetMesh()->GetAnimInstance())
    {
        Anim->Montage_JumpToSection(SEC_Start, GroundSmashMontage);
        Anim->Montage_SetNextSection(SEC_Start, SEC_Loop, GroundSmashMontage);
        Anim->Montage_SetNextSection(SEC_Loop, SEC_Loop, GroundSmashMontage);
    }

    // ��� & ���� ���
    if (auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, GroundSmashMontage, 1.f, NAME_None, false))
    {
        Task->OnCompleted.AddDynamic(this, &UGA_BossPhase2::OnSmashMontageFinished);
        Task->OnBlendOut.AddDynamic(this, &UGA_BossPhase2::OnSmashMontageFinished);
        Task->OnInterrupted.AddDynamic(this, &UGA_BossPhase2::OnSmashMontageFinished);
        Task->OnCancelled.AddDynamic(this, &UGA_BossPhase2::OnSmashMontageFinished);
        Task->ReadyForActivation();
    }

    // ���� �̺�Ʈ ��� (BossCharacter::Landed���� SendGameplayEvent)
    if (auto* Wait = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, LandEventTag))
    {
        Wait->EventReceived.AddDynamic(this, &UGA_BossPhase2::OnLandEvent);
        Wait->ReadyForActivation();
    }
}

void UGA_BossPhase2::OnLandEvent(FGameplayEventData Payload)
{
    ACharacter* C = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (C && GroundSmashMontage)
    {
        if (UAnimInstance* Anim = C->GetMesh()->GetAnimInstance())
        {
            Anim->Montage_SetNextSection(SEC_Loop, SEC_End, GroundSmashMontage);
            Anim->Montage_JumpToSection(SEC_End, GroundSmashMontage);
        }
    }
    
    if (ShockwaveActorClass && C && C->HasAuthority())
    {
        UWorld* World = C->GetWorld();
        const FVector Start = C->GetActorLocation() + FVector(0, 0, 50.f);
        const FVector End = Start + FVector(0, 0, -3000.f);

        FCollisionQueryParams Params(SCENE_QUERY_STAT(ShockwaveGroundTrace), false, C);
        FHitResult Hit;
        FVector SpawnLoc = C->GetActorLocation();
        FRotator SpawnRot = C->GetActorRotation();

        if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
        {
            // �ٴڿ� ��¦ ��� ������� ����
            const float GroundOffset = 2.f;
            SpawnLoc = Hit.ImpactPoint + Hit.Normal * GroundOffset;

            // �����̸� ������ ���� ����(Z)�� ������ ȸ��
            SpawnRot = FRotationMatrix::MakeFromZ(Hit.Normal).Rotator();
        }

        FTransform T(SpawnRot, SpawnLoc);
        if (AActor* Spawned = World->SpawnActor<AActor>(ShockwaveActorClass, T))
        {
            if (AShockwaveActor* SW = Cast<AShockwaveActor>(Spawned))
            {
                UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
                SW->Initialize(
                    SourceASC,
                    /*DamageGE*/      GE_ShockWave /* �÷��̾�� GE�� ��ü ���� */,
                    /*Damage*/        ShockwaveDamage,
                    /*MaxRadius*/     ShockwaveRadius,
                    /*ExpandSpeed*/   5000.f,
                    /*SourceActor*/   C
                );
            }
        }
    }

    //if (ShockwaveActorClass)
    //{
    //    FTransform T(C->GetActorRotation(), C->GetActorLocation());

    //    // ���������� ����/�ʱ�ȭ ����
    //    if (C->HasAuthority())
    //    {
    //        AActor* Spawned = C->GetWorld()->SpawnActor<AActor>(ShockwaveActorClass, T);
    //        if (AShockwaveActor* SW = Cast<AShockwaveActor>(Spawned))
    //        {
    //            UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
    //            SW->Initialize(
    //                SourceASC,
    //                /* InDamageGE   */ GE_ShockWave /* or ���� GE_PlayerDamage */,
    //                /* InDamage     */ ShockwaveDamage,
    //                /* InMaxRadius  */ ShockwaveRadius,
    //                /* InExpandSpd  */ 2500.f,     // Ȯ�� �ӵ�, �ʿ�� Ʃ��
    //                /* InSourceActor*/ C
    //            );
    //        }
    //    }
    //}
}

void UGA_BossPhase2::OnSmashMontageFinished()
{
    bSmashInProgress = false;
    
    if (bShouldEndAfterCurrentSmash && !bEnding)
    {
        BeginEndSequence();
        return;
    }
}

void UGA_BossPhase2::BeginStartSequence()
{
    if (bStartingPhase2) return;
    bStartingPhase2 = true;

    // 1) Hit GA ���
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        FGameplayTagContainer CancelTags;
        CancelTags.AddTag(MonsterTags::Ability_HitReact);
        ASC->CancelAbilities(&CancelTags);
    }

    // 2) ���� ���� ��Ÿ�� ����(����)
    if (ACharacter* C = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
    {
        if (UAnimInstance* Anim = C->GetMesh() ? C->GetMesh()->GetAnimInstance() : nullptr)
        {
            Anim->StopAllMontages(0.10f);
        }
    }

    // 3) ���� ����(����)
    if (StartSound)
    {
        if (ACharacter* Char = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
        {
            UGameplayStatics::SpawnSoundAttached(StartSound, Char->GetMesh());
        }
        else if (AActor* Boss = GetAvatarActorFromActorInfo())
        {
            UGameplayStatics::PlaySoundAtLocation(Boss, StartSound, Boss->GetActorLocation());
        }
    }

    // 4) ���� ƽ���� Start ��Ÿ�� ó��
    if (AActor* Boss = GetAvatarActorFromActorInfo())
    {
        Boss->GetWorldTimerManager().SetTimerForNextTick(this, &UGA_BossPhase2::PlayStartMontageThenStartSmash);
    }
}

void UGA_BossPhase2::PlayStartMontageThenStartSmash()
{
    // StartMontage �̼����̸� �ٷ� ���� ����
    if (!StartMontage)
    {
        SpawnWeakPoints(); // <- ���� ����
        StartSmashLoop();
        return;
    }

    if (auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, StartMontage, 1.f, NAME_None, /*bStopWhenAbilityEnds=*/false))
    {
        // � ���� ���̽��� Smash ������ ����
        Task->OnCompleted.AddDynamic(this, &UGA_BossPhase2::StartSmashLoop);
        Task->OnBlendOut.AddDynamic(this, &UGA_BossPhase2::StartSmashLoop);
        Task->OnInterrupted.AddDynamic(this, &UGA_BossPhase2::StartSmashLoop);
        Task->OnCancelled.AddDynamic(this, &UGA_BossPhase2::StartSmashLoop);
        Task->ReadyForActivation();
        SpawnWeakPoints();
        return;
    }
    else
    {
        // ������
        SpawnWeakPoints();
        StartSmashLoop();
    }
}

void UGA_BossPhase2::StartSmashLoop()
{
    if (bShouldEndAfterCurrentSmash || bEnding)  // End ��û �� ���� ���� ����
    {
        BeginEndSequence();
        return;
    }


    // �̹� Ÿ�̸Ӱ� �ִٸ� �ߺ� ����
    if (AActor* Boss = GetAvatarActorFromActorInfo())
    {
        if (!Boss->GetWorldTimerManager().IsTimerActive(SmashTimerHandle))
        {
            Boss->GetWorldTimerManager().SetTimer(
                SmashTimerHandle, this, &UGA_BossPhase2::SmashTick,
                SmashInterval, true, 1.f /*�ʱ� ����*/);
        }
    }
}

void UGA_BossPhase2::BeginEndSequence()
{
    if (bEnding) return;
    bEnding = true;

    // � ���� ���Ž� ���� ����
    if (AActor* Boss = GetAvatarActorFromActorInfo())
    {
        Boss->GetWorldTimerManager().ClearTimer(SmashTimerHandle);
    }

    // ���Žð� ���� ���̸�(������) ���⼭ ������ ��ٸ����� return ó���� ����
    if (bSmashInProgress)
    {
        // ���������� ��ٸ��� �ʹٸ� �ܼ� return; �ص� ��.
        // ������ ���⼭�� ���Ž� ���� �ݹ鿡���� �������� ������ ���������Ƿ� ��� false.
    }

    // �ǰ� �� ���(���� ���� ��� ����)
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        FGameplayTagContainer CancelTags;
        CancelTags.AddTag(MonsterTags::Ability_HitReact);
        ASC->CancelAbilities(&CancelTags);
    }

    // ���� ���� ���Ž�/��Ÿ ��Ÿ�� ����(����)
    if (ACharacter* C = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
    {
        if (UAnimInstance* Anim = C->GetMesh() ? C->GetMesh()->GetAnimInstance() : nullptr)
        {
            Anim->StopAllMontages(0.10f);
        }
    }

    // ���� ��Ÿ�� ��� �� �Ϸ�Ǹ� EndAbility
    PlayEndMontageAndFinish();
}

void UGA_BossPhase2::PlayEndMontageAndFinish()
{
    if (EndMontage)
    {
        if (auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, NAME_None, EndMontage, 1.f, NAME_None, /*bStopWhenAbilityEnds=*/false))
        {
            Task->OnCompleted.AddDynamic(this, &UGA_BossPhase2::K2_EndAbility);
            Task->OnBlendOut.AddDynamic(this, &UGA_BossPhase2::K2_EndAbility);
            Task->OnInterrupted.AddDynamic(this, &UGA_BossPhase2::K2_EndAbility);
            Task->OnCancelled.AddDynamic(this, &UGA_BossPhase2::K2_EndAbility);
            Task->ReadyForActivation();
            return;
        }
    }

    // ���� ��Ÿ�ְ� ���ų� Task ���� �� ������
    K2_EndAbility();
}

void UGA_BossPhase2::SpawnWeakPoints()
{
    if (!WeakPointClass) return;

    AActor* Boss = GetAvatarActorFromActorInfo();
    if (!Boss) return;

    UWorld* World = Boss->GetWorld();
    if (!World) return;

    UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(World);
    const FVector Origin = Boss->GetActorLocation();

    TArray<FVector> PlacedXYs;
    const float MinDist = 300.f;
    const float MinDistSq = MinDist * MinDist;

    // �ٴ� Ʈ���̽����� ����/�������� ����
    FCollisionQueryParams QP(SCENE_QUERY_STAT(P2_WeakSpawn), false, Boss);
    QP.AddIgnoredActor(Boss);

    for (int32 i = 0; i < WeakPointSpawnCount; ++i)
    {
        // XY �ĺ� ����
        FVector CandidateXY;
        bool bOk = false;

        for (int32 Try = 0; Try < 30; ++Try)
        {
            CandidateXY = RandomOnRing2D(Origin, WeakPointSpawnRadius);

            // Nav ���� XY ���� (Z�� �ŷ� ����)
            if (Nav)
            {
                FNavLocation Out;
                if (Nav->ProjectPointToNavigation(CandidateXY, Out, FVector(200.f)))
                    CandidateXY = Out.Location;
            }

            // �̹� ���� ��ġ��� XY ���� ����
            bOk = true;
            for (const FVector& P : PlacedXYs)
            {
                const float d2 = FMath::Square(P.X - CandidateXY.X) + FMath::Square(P.Y - CandidateXY.Y);
                if (d2 < MinDistSq) { bOk = false; break; }
            }
            if (bOk) break;
        }

        PlacedXYs.Add(CandidateXY);

        // �ٴ����� Z�� ����
        FVector GroundLoc;
        if (!ProjectToGround_NoTilt(World, CandidateXY, GroundLoc, 1500.f, 4000.f, 2.f, ECC_Visibility, &QP))
        {
            // ���� �� ���� �߹� ��ó�� ���� ��ġ
            GroundLoc = FVector(CandidateXY.X, CandidateXY.Y, Origin.Z - 50.f);
        }

        // ȸ���� "��ġ/�� 0", Yaw��(���ϸ� ���� Yaw ���)
        const float Yaw = Boss->GetActorRotation().Yaw; // �Ǵ� 0.f
        const FRotator SpawnRot(0.f, Yaw, 0.f);

        // �����Ʈ���̼� ����: Deferred + �浹 ��Ȱ�� �� ��ġ ���� �� Finish �� �浹 �ѱ�
        FTransform SpawnTM(SpawnRot, GroundLoc);
        AActor* Spawned = World->SpawnActorDeferred<AActor>(
            WeakPointClass,
            SpawnTM,
            Boss,                                // Owner
            nullptr,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

        if (!Spawned) continue;

        // ��Ʈ �浹 ��� OFF (�з� ���� Ƣ�� �� ����)
        if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Spawned->GetRootComponent()))
        {
            RootPrim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }

        // ���⼭ �ʿ��� �ʱ�ȭ ����
        if (AWeakPointActor* WP = Cast<AWeakPointActor>(Spawned))
        {
            WP->InitializeWeakPoint(Boss, WeakPointDamageToBoss);
        }

        // ���� ���� �Ϸ�
        UGameplayStatics::FinishSpawningActor(Spawned, SpawnTM);

        // ��ġ/ȸ�� �� �� �� Ȯ��(���� ���� ���� �ڷ���Ʈ�� ����)
        Spawned->SetActorLocation(GroundLoc, /*bSweep=*/false, nullptr, ETeleportType::TeleportPhysics);
        Spawned->SetActorRotation(SpawnRot, ETeleportType::TeleportPhysics);

        // �浹 ��Ȱ�� (QueryOnly �Ǵ� ������Ʈ ��Ģ�� ���߾� ����)
        if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Spawned->GetRootComponent()))
        {
            RootPrim->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            // �ʿ� �� �������� ����: RootPrim->SetCollisionProfileName(TEXT("WorldDynamic"));
        }
    }

    // �ı� �̺�Ʈ ����(���� �״��)
    if (UAbilityTask_WaitGameplayEvent* Wait =
        UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, MonsterTags::Event_Boss_P2_WeakPointDestroyed, nullptr, false, true))
    {
        Wait->EventReceived.AddDynamic(this, &UGA_BossPhase2::OnWeakPointDestroyedEvent);
        Wait->ReadyForActivation();
    }
}

void UGA_BossPhase2::OnWeakPointDestroyedEvent(FGameplayEventData Payload)
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (!ASC) return;

    float Damage = WeakPointDamageToBoss;
    if (Payload.EventMagnitude > 0.f)
    {
        Damage = Payload.EventMagnitude; // ���ͺ� Ŀ���� ���� ���
    }

    if (GE_WeakPointDamageToBoss)
    {
        FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
        // Instigator�� ���� ���ͷ� ����(����)
        // Instigator: ����(�ڱ� �ڽ�)�� �δ� ���� ����
        AActor* BossActor = GetAvatarActorFromActorInfo();

        // EffectCauser: ���� ����(�̺�Ʈ ���� ��)�� �ְ� ������ const_cast
        AActor* EffectCauser = nullptr;
        if (Payload.Instigator)
        {
            EffectCauser = const_cast<AActor*>(Payload.Instigator.Get());
        }

        // ������ ���(Self-apply)�̹Ƿ� �̷��� ���� 
        Ctx.AddInstigator(BossActor, EffectCauser);

        FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(GE_WeakPointDamageToBoss, 1.f, Ctx);
        if (Spec.IsValid())
        {
            Spec.Data->SetSetByCallerMagnitude(MonsterTags::Data_Boss_Damaged, Damage);
            ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
        }
    }
}


void UGA_BossPhase2::ApplyInvuln()
{
    if (InvulnHandle.IsValid()) return;

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    AActor* Boss = GetAvatarActorFromActorInfo();
    if (!ASC || !Boss || !GE_BossInvuln) return;

    FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
    Ctx.AddInstigator(Boss, Boss->GetInstigatorController());

    FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(GE_BossInvuln, 1.f, Ctx);
    if (Spec.IsValid())
    {
        InvulnHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
        // ����: GE_BossInvuln ���ο� GameplayCue(���� VFX)�� ���õǾ� ������ �ڵ� ����˴ϴ�.
    }
}

void UGA_BossPhase2::RemoveInvuln()
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (ASC && InvulnHandle.IsValid())
    {
        ASC->RemoveActiveGameplayEffect(InvulnHandle);
        InvulnHandle.Invalidate();
    }
}

void UGA_BossPhase2::OnEndMontageFinished()
{
    RemoveInvuln();

    // �׸��� ���� ����
    K2_EndAbility();
}