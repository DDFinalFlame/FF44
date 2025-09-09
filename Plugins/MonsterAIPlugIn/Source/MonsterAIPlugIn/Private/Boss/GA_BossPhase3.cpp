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
    const FVector& XY,                   // X,Y만 의미 있음
    FVector& OutGroundLoc,               // 최종 스폰 위치(Z 확정)
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

    // 필요시 시작/차단 태그 세팅 가능
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

    // 1) 낙석 루프
    W->GetTimerManager().SetTimer(RockTimer, this, &UGA_BossPhase3::Tick_Rock, RockInterval, /*bLoop*/true, /*FirstDelay*/0.f);

    // 2) 공격 루프 (가변 간격)
    const float FirstAtkDelay = FMath::FRandRange(AttackIntervalMin, AttackIntervalMax);
    W->GetTimerManager().SetTimer(AttackTimer, this, &UGA_BossPhase3::Tick_Attack, FirstAtkDelay, /*bLoop*/false);

    // 3) 미니언 루프
    W->GetTimerManager().SetTimer(MinionTimer, this, &UGA_BossPhase3::Tick_Minion, MinionInterval, /*bLoop*/true, /*FirstDelay*/2.f);

    // 약점 파괴 이벤트 수신 (Phase2와 동일 태그 사용)
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
    const FVector SpawnLoc = XY + FVector(0, 0, 1200.f);            // 위에서 낙하
    Boss->GetWorld()->SpawnActor<AActor>(FallingRockClass, FTransform(SpawnLoc));
}

void UGA_BossPhase3::Tick_Attack()
{
    ACharacter* Boss = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Boss) return;

    if (bAttackPlaying)
    {
        // 이미 재생 중이면 다음 예약만
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


        // Deferred Spawn으로 BeginPlay 전에 안전하게 주입
        ACharacter* Spawned = World->SpawnActorDeferred<ACharacter>(
            MinionClass, SpawnTM, /*Owner=*/Boss, /*Instigator=*/Boss,
            ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

        if (!Spawned) continue;

        if (AMonsterCharacter* MC = Cast<AMonsterCharacter>(Spawned))
        {
            MC->SetOwnerBoss(Boss); //  보스 주입!
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
    // Phase2와 동일하게 약점 파괴 → 보스 HP 감소 처리
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
    bDisableWeakpointSpawns = true;   // 이후 약점 스폰 가드
    ForceKillPhaseMinions();          // 남은 미니언들 정리

    // 타이머/이동/포커스 정지
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

        // 진행 중인 몽타주 중지(충돌 방지)
        if (UAnimInstance* Anim = Boss->GetMesh() ? Boss->GetMesh()->GetAnimInstance() : nullptr)
        {
            Anim->StopAllMontages(0.1f);
        }
    }

    RemoveInvuln();   // 무적 해제
    UnbindHP();       // HP 델리게이트 해제

    // (선택) HitReact 등 방해되는 GA 취소, Death GA만 남기기
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        // Death GA 태그는 프로젝트에 맞게 교체
        FGameplayTagContainer Ignore;
        // Ignore.AddTag(MonsterTags::Ability_Death);

        ASC->CancelAbilities(/*WithTags=*/nullptr, /*WithoutTags=*/Ignore.Num() ? &Ignore : nullptr);
    }

    // (선택) Death GA 트리거 (프로젝트 규칙에 맞게 이벤트/GE/태그로 호출)
    if (bTriggerDeathGA)
    {
        if (AActor* Boss = GetAvatarActorFromActorInfo())
        {
            // 예: 이벤트 방식
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

    // 파라미터
    const float Rmin = 350.f;
    const float Rmax = 900.f;
    const int32 MaxTries = 24;
    const float PlaceCheckRadius = 80.f;     // 자리 비었는지 검사 반경
    const float PlacementLift = 40.f;        // 바닥과의 겹침 판정 회피용 리프트
    const ECollisionChannel GroundChannel = ECC_Visibility;

    // 바닥/자리 체크 시 무시할 대상
    FCollisionQueryParams QP(SCENE_QUERY_STAT(P3_SpawnWeakPoint), false, Boss);
    QP.AddIgnoredActor(Boss);

    if (const AActor* DeadMinion =
        Payload.Target.Get() ? Payload.Target.Get() :
        (Payload.Instigator.Get() ? Payload.Instigator.Get() :
            Cast<AActor>(Payload.OptionalObject)))
    {
        QP.AddIgnoredActor(DeadMinion);
    }

    // "자리 비었는지"는 바닥(WorldStatic)은 제외하고,
    // Pawn/WorldDynamic/PhysicsBody 등만 대상으로 오버랩 검사
    FCollisionObjectQueryParams ObjParams;
    ObjParams.AddObjectTypesToQuery(ECC_Pawn);
    ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);
    ObjParams.AddObjectTypesToQuery(ECC_PhysicsBody);

    // 1) XY 샘플링 → 바닥으로 Z만 투영 → 다른 액터와 겹치지 않는지 검사
    FVector SpawnLoc;
    bool bFound = false;

    for (int32 i = 0; i < MaxTries; ++i)
    {
        const float r = FMath::Sqrt(FMath::FRandRange(Rmin * Rmin, Rmax * Rmax));
        const float th = FMath::FRandRange(0.f, 2.f * PI);
        const FVector CandidateXY = Center + FVector(r * FMath::Cos(th), r * FMath::Sin(th), 0.f);

        // Z만 바닥 투영
        FVector GroundLoc;
        if (!ProjectToGround_NoTilt(World, CandidateXY, GroundLoc, 2000.f, 6000.f, 2.f, GroundChannel, &QP))
            continue;

        // 바닥은 제외하고 "다른 액터"와 겹치지 않는지 검사
        const FCollisionShape Sphere = FCollisionShape::MakeSphere(PlaceCheckRadius);
        const bool bOverlapsOthers = World->OverlapAnyTestByObjectType(
            GroundLoc + FVector(0, 0, PlacementLift), // 살짝 띄워 검사
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

    // 2) 실패 시: 보스 발치 조금 앞 XY로 폴백하되, 반드시 바닥 투영으로 Z 확정
    if (!bFound)
    {
        const FVector FallbackXY = Center + FVector(200.f, 0.f, 0.f);
        if (!ProjectToGround_NoTilt(World, FallbackXY, SpawnLoc, 2000.f, 6000.f, 2.f, GroundChannel, &QP))
        {
            // 최후의 안전망: Z만 보스보다 조금 낮춰서 박기
            SpawnLoc = FVector(FallbackXY.X, FallbackXY.Y, Center.Z - 50.f);
        }
    }

    // 3) 틸팅 금지: 피치/롤 0, Yaw는 보스와 동일(원치 않으면 0.f)
    const float Yaw = Boss->GetActorRotation().Yaw;
    const FRotator SpawnRot(0.f, Yaw, 0.f);
    const FTransform SpawnTM(SpawnRot, SpawnLoc);

    // 4) Defer + NoCollision → Finish → 위치/회전 텔레포트 확정 → 충돌 재활성
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

    // 스폰 직후 위치/회전 한 번 더 확정(물리 텔레포트)
    Spawned->SetActorLocation(SpawnLoc, /*bSweep=*/false, nullptr, ETeleportType::TeleportPhysics);
    Spawned->SetActorRotation(SpawnRot, ETeleportType::TeleportPhysics);

    // 충돌 재활성 (프로젝트 규칙에 맞춰 조정)
    if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Spawned->GetRootComponent()))
    {
        RootPrim->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        // 필요 시 프로파일 지정:
        // RootPrim->SetCollisionProfileName(TEXT("WorldDynamic"));
    }
}

//void UGA_BossPhase3::OnMinionDied(FGameplayEventData Payload)
//{
//    AActor* Boss = GetAvatarActorFromActorInfo();
//    UWorld* World = Boss ? Boss->GetWorld() : nullptr;
//    if (!World || !WeakPointClass) return;
//
//    // 1) 죽은 미니언 액터 추출 (Target → Instigator → OptionalObject 순)
//    AActor* DeadMinion = const_cast<AActor*>(
//        Payload.Target.Get() ? Payload.Target.Get() :
//        (Payload.Instigator.Get() ? Payload.Instigator.Get() :
//            Cast<AActor>(Payload.OptionalObject))
//        );
//
//    if (!DeadMinion) DeadMinion = Boss; // 안전망: 없으면 보스 주변
//
//    // 2) 파라미터(간단)
//    const float Rmin = 350.f;
//    const float Rmax = 900.f;
//    const int32 MaxTries = 24;
//    const float GroundTraceUp = 1200.f;
//    const float GroundTraceDown = 3000.f;
//    const float PlaceCheckRadius = 80.f;   // 스폰물 대략 반경
//    const ECollisionChannel GroundChannel = ECC_Visibility;
//    const ECollisionChannel PlaceCheckChannel = ECC_Pawn; // 필요시 커스텀 채널로 교체
//
//    // 충돌 무시 목록
//    FCollisionQueryParams QP(SCENE_QUERY_STAT(P3_SpawnWeakPoint), false, Boss);
//    QP.AddIgnoredActor(Boss);
//    QP.AddIgnoredActor(DeadMinion);
//
//    // 3) 후보 샘플링 → 바닥 트레이스 → 자리 비었는지 구 스윕
//    auto TryFindSpot = [&](FVector& OutLoc, FRotator& OutRot)->bool
//        {
//            for (int32 i = 0; i < MaxTries; ++i)
//            {
//                const float r = FMath::Sqrt(FMath::FRandRange(Rmin * Rmin, Rmax * Rmax));
//                const float th = FMath::FRandRange(0.f, 2.f * PI);
//                const FVector XY = DeadMinion->GetActorLocation() + FVector(r * FMath::Cos(th), r * FMath::Sin(th), 0.f);
//
//                // 바닥 찾기
//                FHitResult Hit;
//                const FVector Start = XY + FVector(0, 0, GroundTraceUp);
//                const FVector End = XY - FVector(0, 0, GroundTraceDown);
//                if (!World->LineTraceSingleByChannel(Hit, Start, End, GroundChannel, QP)) continue;
//
//                // 살짝 띄우고 경사 정렬
//                const FVector Loc = Hit.ImpactPoint + Hit.Normal * 2.f;
//                const FRotator Rot = FRotationMatrix::MakeFromZ(Hit.Normal).Rotator();
//
//                // 자리 비었는지(구 스윕)
//                const FCollisionShape Sphere = FCollisionShape::MakeSphere(PlaceCheckRadius);
//                FHitResult Block;
//                const FVector P = Loc + FVector(0, 0, 50.f); // 살짝 위에서 정지 스윕
//                const bool bBlocked = World->SweepSingleByChannel(Block, P, P, FQuat::Identity, PlaceCheckChannel, Sphere, QP);
//                if (!bBlocked) { OutLoc = Loc; OutRot = Rot; return true; }
//            }
//            return false;
//        };
//
//    FVector SpawnLoc;  FRotator SpawnRot;
//    if (!TryFindSpot(SpawnLoc, SpawnRot))
//    {
//        // 4) 마지막 안전망: 미니언 위치 조금 옆
//        SpawnLoc = DeadMinion->GetActorLocation() + FVector(200.f, 0.f, -30.f);
//        SpawnRot = FRotator::ZeroRotator;
//    }
//
//    // 5) 스폰 & 초기화
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
        // 타겟이 없으면 잠깐 뒤 재시도
        const float Delay = FMath::FRandRange(0.5f, 1.0f);
        Boss->GetWorldTimerManager().SetTimer(AttackTimer, this, &UGA_BossPhase3::Tick_Attack, Delay, false);
        return;
    }

    const float DistXY = FVector::Dist2D(Boss->GetActorLocation(), Target->GetActorLocation());
    if (DistXY <= ApproachAcceptanceRadius)
    {
        // 이미 도착 범위 → 곧바로 공격
        StartRandomAttackMontageOrReschedule();
        return;
    }

    AAIController* AI = Cast<AAIController>(Boss->GetController());
    if (!AI)
    {
        // 컨트롤러 없으면 바로 공격(최소한으로 동작 보장)
        StartRandomAttackMontageOrReschedule();
        return;
    }

    // 기존 델리게이트 중복 방지
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

    // 결과가 바로 판정나는 경우 처리
    if (R.Code == EPathFollowingRequestResult::AlreadyAtGoal)
    {
        bMovingToAttack = false;
        StartRandomAttackMontageOrReschedule();
        return;
    }
    if (R.Code == EPathFollowingRequestResult::Failed)
    {
        bMovingToAttack = false;
        // 이동 실패 → 짧게 쉬고 재시도
        const float Delay = FMath::FRandRange(0.6f, 1.2f);
        Boss->GetWorldTimerManager().SetTimer(AttackTimer, this, &UGA_BossPhase3::Tick_Attack, Delay, false);
        return;
    }

    // 정상적으로 이동 시작 → 완료 콜백 바인딩
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

    // 내가 요청한 MoveTo가 아니면 무시(다른 시스템과 충돌 방지)
    if (RequestID.IsValid() && CurrentMoveId.IsValid() && RequestID != CurrentMoveId)
    {
        return;
    }

    // 도착했으면 공격, 실패면 재시도
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

    // 사용할 몽타주 수집
    TArray<UAnimMontage*> Candidates;
    for (UAnimMontage* M : AttackMontages)
    {
        if (M) Candidates.Add(M);
    }

    if (Candidates.Num() == 0)
    {
        // 몽타주 없으면 다음 시도 예약
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
    // 서버에서만 적용
    AActor* Boss = GetAvatarActorFromActorInfo();
    if (!Boss || !Boss->HasAuthority())
    {
        return;
    }

    if (!GE_ForceKillMinion)
    {
        UE_LOG(LogTemp, Warning, TEXT("[P3] GE_ForceKillMinion is not set."));
        // 그래도 배열만 정리하고 종료
        PhaseMinions.Reset();
        return;
    }

    // 뒤에서부터 안전하게 제거
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

        // 이미 죽었거나(HP<=0) Death 처리 중이면 스킵
        const float CurHP = MinionASC->GetNumericAttribute(UMonsterAttributeSet::GetHealthAttribute());
        if (CurHP <= 0.f)
        {
            PhaseMinions.RemoveAt(i);
            continue;
        }

        // Health를 0으로 만드는 전용 GE 적용
        {
            FGameplayEffectContextHandle Ctx = MinionASC->MakeEffectContext();
            Ctx.AddInstigator(Boss, Boss); // 출처: 보스

            FGameplayEffectSpecHandle Spec = MinionASC->MakeOutgoingSpec(GE_ForceKillMinion, 1.f, Ctx);
            if (Spec.IsValid())
            {
                MinionASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
            }
        }

        // 목록에서 제거 (중복 적용 방지)
        PhaseMinions.RemoveAt(i);
    }

    // 안전하게 비워두기
    PhaseMinions.Shrink();
}