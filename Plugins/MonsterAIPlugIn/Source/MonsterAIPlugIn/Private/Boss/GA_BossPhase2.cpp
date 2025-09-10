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
    const FVector& XY,                   // X,Y만 의미 있음
    FVector& OutGroundLoc,               // 최종 스폰 위치(Z 확정)
    float TraceUp = 1500.f,
    float TraceDown = 4000.f,
    float GroundOffset = 2.f,            // 지프라깅 방지
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
        OutGroundLoc = Hit.ImpactPoint + FVector(0, 0, GroundOffset); // Z만 보정
        return true;
    }
    return false;
}

UGA_BossPhase2::UGA_BossPhase2()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    // 태그는 프로젝트 규칙에 맞게 조정하세요
    FGameplayTagContainer AssetTags;
    AssetTags.AddTag(MonsterTags::Ability_Boss_PhaseStart);
    // 선택: 개별 식별 태그도 있으면 추가
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
        // 타이머 미가동 상태 보장
        if (AActor* Boss = GetAvatarActorFromActorInfo())
        {
            Boss->GetWorldTimerManager().ClearTimer(SmashTimerHandle);
        }
        // 아직 스매시가 없으니 바로 엔딩
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

        // End 조건: 항상 체크
        if (Ratio <= EndHpRatioThreshold && !bShouldEndAfterCurrentSmash && !bEnding)
        {
            bShouldEndAfterCurrentSmash = true;

            if (AActor* Boss = GetAvatarActorFromActorInfo())
            {
                Boss->GetWorldTimerManager().ClearTimer(SmashTimerHandle); // 새 스매시 방지
            }

            if (!bSmashInProgress)
            {
                BeginEndSequence(); // 스매시 중이 아니면 바로 엔딩
                return;
            }
            // 스매시 중이면 OnSmashMontageFinished에서 엔딩 진입
        }

        // Start 조건: 아직 Phase 시작 안 했을 때만
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
        const float TargetHP = MaxHP * EndHpRatioThreshold;  // 예: 0.20 → 20%

        const float Need = FMath::Max(0.f, CurHP - TargetHP);   // 지금부터 목표까지 깎아야 할 총량
        const int32 Count = FMath::Max(1, WeakPointSpawnCount); // 0 나눗셈 방지

        float perWeak = Need / Count;

        // “혹시 모르니까 1 정도만 키우자”
        if (Need > KINDA_SMALL_NUMBER)
        {
            perWeak += 1.f;
        }

        // 요청하신 대로 ‘음수’로 저장(Health를 Add로 줄이는 GE라면 음수여야 체력이 감소)
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

    // 진행 중인 시작/스매시 몽타주가 있다면 안전하게 중지
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
        // 캐스팅/소환 페이즈 종료 → 공격 페이즈로
        Boss->SetBossState_EBB((uint8)EBossState_BB::InPhase3);
    }

    RemoveInvuln();

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        if (GA_BossPhase3Class) // UPROPERTY로 GA_BossPhase3Class를 받아두면 됨
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
        // 현재 스매시를 시작하지 않고 엔딩으로
        BeginEndSequence();
        return;
    }


    if (bSmashInProgress) return; // 이전 쿵이 아직 진행 중이면 스킵
    bSmashInProgress = true;

    ACharacter* C = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!C || !GroundSmashMontage) { bSmashInProgress = false; return; }

    // 점프(루트모션으로 점프하면 Launch 생략)
    if (JumpZ > 0.f) C->LaunchCharacter(FVector(0, 0, JumpZ), false, true);

    // 몽타주: Start→Loop(무한) 설정
    if (UAnimInstance* Anim = C->GetMesh()->GetAnimInstance())
    {
        Anim->Montage_JumpToSection(SEC_Start, GroundSmashMontage);
        Anim->Montage_SetNextSection(SEC_Start, SEC_Loop, GroundSmashMontage);
        Anim->Montage_SetNextSection(SEC_Loop, SEC_Loop, GroundSmashMontage);
    }

    // 재생 & 종료 대기
    if (auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, GroundSmashMontage, 1.f, NAME_None, false))
    {
        Task->OnCompleted.AddDynamic(this, &UGA_BossPhase2::OnSmashMontageFinished);
        Task->OnBlendOut.AddDynamic(this, &UGA_BossPhase2::OnSmashMontageFinished);
        Task->OnInterrupted.AddDynamic(this, &UGA_BossPhase2::OnSmashMontageFinished);
        Task->OnCancelled.AddDynamic(this, &UGA_BossPhase2::OnSmashMontageFinished);
        Task->ReadyForActivation();
    }

    // 착지 이벤트 대기 (BossCharacter::Landed에서 SendGameplayEvent)
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
            // 바닥에 살짝 띄워 지프라깅 방지
            const float GroundOffset = 2.f;
            SpawnLoc = Hit.ImpactPoint + Hit.Normal * GroundOffset;

            // 경사면이면 법선에 맞춰 위쪽(Z)이 서도록 회전
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
                    /*DamageGE*/      GE_ShockWave /* 플레이어용 GE로 교체 권장 */,
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

    //    // 서버에서만 스폰/초기화 권장
    //    if (C->HasAuthority())
    //    {
    //        AActor* Spawned = C->GetWorld()->SpawnActor<AActor>(ShockwaveActorClass, T);
    //        if (AShockwaveActor* SW = Cast<AShockwaveActor>(Spawned))
    //        {
    //            UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
    //            SW->Initialize(
    //                SourceASC,
    //                /* InDamageGE   */ GE_ShockWave /* or 전용 GE_PlayerDamage */,
    //                /* InDamage     */ ShockwaveDamage,
    //                /* InMaxRadius  */ ShockwaveRadius,
    //                /* InExpandSpd  */ 2500.f,     // 확산 속도, 필요시 튜닝
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

    // 1) Hit GA 취소
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        FGameplayTagContainer CancelTags;
        CancelTags.AddTag(MonsterTags::Ability_HitReact);
        ASC->CancelAbilities(&CancelTags);
    }

    // 2) 진행 중인 몽타주 정지(안전)
    if (ACharacter* C = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
    {
        if (UAnimInstance* Anim = C->GetMesh() ? C->GetMesh()->GetAnimInstance() : nullptr)
        {
            Anim->StopAllMontages(0.10f);
        }
    }

    // 3) 시작 사운드(선택)
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

    // 4) 다음 틱에서 Start 몽타주 처리
    if (AActor* Boss = GetAvatarActorFromActorInfo())
    {
        Boss->GetWorldTimerManager().SetTimerForNextTick(this, &UGA_BossPhase2::PlayStartMontageThenStartSmash);
    }
}

void UGA_BossPhase2::PlayStartMontageThenStartSmash()
{
    // StartMontage 미설정이면 바로 루프 시작
    if (!StartMontage)
    {
        SpawnWeakPoints(); // <- 약점 스폰
        StartSmashLoop();
        return;
    }

    if (auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, StartMontage, 1.f, NAME_None, /*bStopWhenAbilityEnds=*/false))
    {
        // 어떤 종료 케이스든 Smash 루프로 진입
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
        // 안전망
        SpawnWeakPoints();
        StartSmashLoop();
    }
}

void UGA_BossPhase2::StartSmashLoop()
{
    if (bShouldEndAfterCurrentSmash || bEnding)  // End 요청 시 루프 시작 금지
    {
        BeginEndSequence();
        return;
    }


    // 이미 타이머가 있다면 중복 방지
    if (AActor* Boss = GetAvatarActorFromActorInfo())
    {
        if (!Boss->GetWorldTimerManager().IsTimerActive(SmashTimerHandle))
        {
            Boss->GetWorldTimerManager().SetTimer(
                SmashTimerHandle, this, &UGA_BossPhase2::SmashTick,
                SmashInterval, true, 1.f /*초기 지연*/);
        }
    }
}

void UGA_BossPhase2::BeginEndSequence()
{
    if (bEnding) return;
    bEnding = true;

    // 어떤 경우든 스매시 루프 정지
    if (AActor* Boss = GetAvatarActorFromActorInfo())
    {
        Boss->GetWorldTimerManager().ClearTimer(SmashTimerHandle);
    }

    // 스매시가 진행 중이면(안전망) 여기서 끝까지 기다리려면 return 처리도 가능
    if (bSmashInProgress)
    {
        // 보수적으로 기다리고 싶다면 단순 return; 해도 됨.
        // 하지만 여기서는 스매시 종료 콜백에서만 들어오도록 위에서 통제했으므로 통상 false.
    }

    // 피격 등 취소(엔딩 방해 요소 제거)
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        FGameplayTagContainer CancelTags;
        CancelTags.AddTag(MonsterTags::Ability_HitReact);
        ASC->CancelAbilities(&CancelTags);
    }

    // 진행 중인 스매시/기타 몽타주 정지(안전)
    if (ACharacter* C = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
    {
        if (UAnimInstance* Anim = C->GetMesh() ? C->GetMesh()->GetAnimInstance() : nullptr)
        {
            Anim->StopAllMontages(0.10f);
        }
    }

    // 엔딩 몽타주 재생 → 완료되면 EndAbility
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

    // 엔딩 몽타주가 없거나 Task 실패 시 안전망
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

    // 바닥 트레이스에서 보스/약점끼리 무시
    FCollisionQueryParams QP(SCENE_QUERY_STAT(P2_WeakSpawn), false, Boss);
    QP.AddIgnoredActor(Boss);

    for (int32 i = 0; i < WeakPointSpawnCount; ++i)
    {
        // XY 후보 선정
        FVector CandidateXY;
        bool bOk = false;

        for (int32 Try = 0; Try < 30; ++Try)
        {
            CandidateXY = RandomOnRing2D(Origin, WeakPointSpawnRadius);

            // Nav 위로 XY 보정 (Z는 신뢰 금지)
            if (Nav)
            {
                FNavLocation Out;
                if (Nav->ProjectPointToNavigation(CandidateXY, Out, FVector(200.f)))
                    CandidateXY = Out.Location;
            }

            // 이미 뽑힌 위치들과 XY 간격 유지
            bOk = true;
            for (const FVector& P : PlacedXYs)
            {
                const float d2 = FMath::Square(P.X - CandidateXY.X) + FMath::Square(P.Y - CandidateXY.Y);
                if (d2 < MinDistSq) { bOk = false; break; }
            }
            if (bOk) break;
        }

        PlacedXYs.Add(CandidateXY);

        // 바닥으로 Z만 투영
        FVector GroundLoc;
        if (!ProjectToGround_NoTilt(World, CandidateXY, GroundLoc, 1500.f, 4000.f, 2.f, ECC_Visibility, &QP))
        {
            // 실패 시 보스 발밑 근처로 안전 배치
            GroundLoc = FVector(CandidateXY.X, CandidateXY.Y, Origin.Z - 50.f);
        }

        // 회전은 "피치/롤 0", Yaw만(원하면 보스 Yaw 사용)
        const float Yaw = Boss->GetActorRotation().Yaw; // 또는 0.f
        const FRotator SpawnRot(0.f, Yaw, 0.f);

        // 디페네트레이션 방지: Deferred + 충돌 비활성 → 위치 세팅 → Finish 후 충돌 켜기
        FTransform SpawnTM(SpawnRot, GroundLoc);
        AActor* Spawned = World->SpawnActorDeferred<AActor>(
            WeakPointClass,
            SpawnTM,
            Boss,                                // Owner
            nullptr,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

        if (!Spawned) continue;

        // 루트 충돌 잠시 OFF (밀려 위로 튀는 것 방지)
        if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Spawned->GetRootComponent()))
        {
            RootPrim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }

        // 여기서 필요한 초기화 먼저
        if (AWeakPointActor* WP = Cast<AWeakPointActor>(Spawned))
        {
            WP->InitializeWeakPoint(Boss, WeakPointDamageToBoss);
        }

        // 실제 스폰 완료
        UGameplayStatics::FinishSpawningActor(Spawned, SpawnTM);

        // 위치/회전 한 번 더 확정(스폰 직후 물리 텔레포트로 고정)
        Spawned->SetActorLocation(GroundLoc, /*bSweep=*/false, nullptr, ETeleportType::TeleportPhysics);
        Spawned->SetActorRotation(SpawnRot, ETeleportType::TeleportPhysics);

        // 충돌 재활성 (QueryOnly 또는 프로젝트 규칙에 맞추어 세팅)
        if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Spawned->GetRootComponent()))
        {
            RootPrim->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            // 필요 시 프로파일 지정: RootPrim->SetCollisionProfileName(TEXT("WorldDynamic"));
        }
    }

    // 파괴 이벤트 수신(기존 그대로)
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
        Damage = Payload.EventMagnitude; // 액터별 커스텀 피해 허용
    }

    if (GE_WeakPointDamageToBoss)
    {
        FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
        // Instigator를 약점 액터로 세팅(선택)
        // Instigator: 보스(자기 자신)로 두는 편이 안전
        AActor* BossActor = GetAvatarActorFromActorInfo();

        // EffectCauser: 약점 액터(이벤트 보낸 쪽)를 넣고 싶으면 const_cast
        AActor* EffectCauser = nullptr;
        if (Payload.Instigator)
        {
            EffectCauser = const_cast<AActor*>(Payload.Instigator.Get());
        }

        // 보스가 대상(Self-apply)이므로 이렇게 세팅 
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
        // 주의: GE_BossInvuln 내부에 GameplayCue(쉴드 VFX)가 세팅되어 있으면 자동 재생됩니다.
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

    // 그리고 최종 종료
    K2_EndAbility();
}