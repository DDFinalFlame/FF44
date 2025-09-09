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
    AActor* Boss = GetAvatarActorFromActorInfo();
    UWorld* World = Boss ? Boss->GetWorld() : nullptr;
    if (!World || !WeakPointClass) return;

    // 1) 죽은 미니언 액터 추출 (Target → Instigator → OptionalObject 순)
    AActor* DeadMinion = const_cast<AActor*>(
        Payload.Target.Get() ? Payload.Target.Get() :
        (Payload.Instigator.Get() ? Payload.Instigator.Get() :
            Cast<AActor>(Payload.OptionalObject))
        );

    if (!DeadMinion) DeadMinion = Boss; // 안전망: 없으면 보스 주변

    // 2) 파라미터(간단)
    const float Rmin = 350.f;
    const float Rmax = 900.f;
    const int32 MaxTries = 24;
    const float GroundTraceUp = 1200.f;
    const float GroundTraceDown = 3000.f;
    const float PlaceCheckRadius = 80.f;   // 스폰물 대략 반경
    const ECollisionChannel GroundChannel = ECC_Visibility;
    const ECollisionChannel PlaceCheckChannel = ECC_Pawn; // 필요시 커스텀 채널로 교체

    // 충돌 무시 목록
    FCollisionQueryParams QP(SCENE_QUERY_STAT(P3_SpawnWeakPoint), false, Boss);
    QP.AddIgnoredActor(Boss);
    QP.AddIgnoredActor(DeadMinion);

    // 3) 후보 샘플링 → 바닥 트레이스 → 자리 비었는지 구 스윕
    auto TryFindSpot = [&](FVector& OutLoc, FRotator& OutRot)->bool
        {
            for (int32 i = 0; i < MaxTries; ++i)
            {
                const float r = FMath::Sqrt(FMath::FRandRange(Rmin * Rmin, Rmax * Rmax));
                const float th = FMath::FRandRange(0.f, 2.f * PI);
                const FVector XY = DeadMinion->GetActorLocation() + FVector(r * FMath::Cos(th), r * FMath::Sin(th), 0.f);

                // 바닥 찾기
                FHitResult Hit;
                const FVector Start = XY + FVector(0, 0, GroundTraceUp);
                const FVector End = XY - FVector(0, 0, GroundTraceDown);
                if (!World->LineTraceSingleByChannel(Hit, Start, End, GroundChannel, QP)) continue;

                // 살짝 띄우고 경사 정렬
                const FVector Loc = Hit.ImpactPoint + Hit.Normal * 2.f;
                const FRotator Rot = FRotationMatrix::MakeFromZ(Hit.Normal).Rotator();

                // 자리 비었는지(구 스윕)
                const FCollisionShape Sphere = FCollisionShape::MakeSphere(PlaceCheckRadius);
                FHitResult Block;
                const FVector P = Loc + FVector(0, 0, 50.f); // 살짝 위에서 정지 스윕
                const bool bBlocked = World->SweepSingleByChannel(Block, P, P, FQuat::Identity, PlaceCheckChannel, Sphere, QP);
                if (!bBlocked) { OutLoc = Loc; OutRot = Rot; return true; }
            }
            return false;
        };

    FVector SpawnLoc;  FRotator SpawnRot;
    if (!TryFindSpot(SpawnLoc, SpawnRot))
    {
        // 4) 마지막 안전망: 미니언 위치 조금 옆
        SpawnLoc = DeadMinion->GetActorLocation() + FVector(200.f, 0.f, -30.f);
        SpawnRot = FRotator::ZeroRotator;
    }

    // 5) 스폰 & 초기화
    if (AActor* Spawned = World->SpawnActor<AActor>(WeakPointClass, FTransform(SpawnRot, SpawnLoc)))
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