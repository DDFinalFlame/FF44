// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_Boss_Grab.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "Animation/AnimMontage.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "MotionWarpingComponent.h"
#include "TimerManager.h"
#include "MonsterTags.h"

UGA_Boss_Grab::UGA_Boss_Grab()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly; 
  
    ActivationBlockedTags.AddTag(MonsterTags::State_Boss_Grab_CoolTime);
}

void UGA_Boss_Grab::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{

    // Commit(쿨다운/비용/태그 등)
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    ACharacter* Boss = Cast<ACharacter>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr);
    ACharacter* Victim = nullptr;

    if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
        if (GrabBusyTag.IsValid()) ASC->AddLooseGameplayTag(GrabBusyTag);

    // BT 블랙보드도 true로 (데코레이터에서 쓰기)
    if (AAIController* AI = Cast<AAIController>(Boss->GetController()))
        if (UBlackboardComponent* BB = AI->GetBlackboardComponent())
            if (!BB_CinematicLockKey.IsNone()) BB->SetValueAsBool(BB_CinematicLockKey, true);

    if (TriggerEventData && TriggerEventData->Target)
    {
        const ACharacter* VictimConst = Cast<const ACharacter>(TriggerEventData->Target);
        Victim = const_cast<ACharacter*>(VictimConst);
    }

    if (!Boss || !Victim || !BossGrabMontage)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    CachedBoss = Boss;
    CachedVictim = Victim;

    // 1) (선택) BB에 Victim 기록 → ANS가 BB로 Victim을 획득할 수 있게
    if (AAIController* AI = Cast<AAIController>(Boss->GetController()))
    {
        if (UBlackboardComponent* BB = AI->GetBlackboardComponent())
        {
            if (!BB_TargetActorKey.IsNone())
            {
                BB->SetValueAsObject(BB_TargetActorKey, Victim);
            }
        }
    }

  //   2) (선택) Motion Warping 타깃 세팅
    if (bUseMotionWarping)
    {
        if (UMotionWarpingComponent* MW = Boss->FindComponentByClass<UMotionWarpingComponent>())
        {
            if (Victim->GetMesh())
            {
                const FTransform T = Victim->GetMesh()->GetSocketTransform(VictimSocketName, RTS_World);
                MW->AddOrUpdateWarpTargetFromLocationAndRotation(WarpTargetName, T.GetLocation(), T.Rotator());
            }
        }
    }

    if (bApproachBeforeGrab)
    {
        const float Dist = FVector::Dist2D(Boss->GetActorLocation(), Victim->GetActorLocation());
        if (Dist > ApproachAcceptanceRadius)
        {
            if (AAIController* AI = Cast<AAIController>(Boss->GetController()))
            {
                BeginApproach(AI, Boss, Victim);
                return; // Move 완료/타임아웃 콜백에서 StartGrabMontage 호출
            }
        }
    }

    // 거리 충분하면 곧바로 몽타주 시작
    StartGrabMontage();
}

void UGA_Boss_Grab::OnMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, /*bWasCancelled*/false);
}

void UGA_Boss_Grab::OnMontageCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, /*bWasCancelled*/true);
}

void UGA_Boss_Grab::EndAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    // 접근 타이머/바인딩 정리 있었다면 호출
    CleanupApproachBindings();



    // 1) 태그 해제
    if (UAbilitySystemComponent* ASC = CurrentActorInfo ? CurrentActorInfo->AbilitySystemComponent.Get() : nullptr)
    {
        if (GrabBusyTag.IsValid() && ASC->HasMatchingGameplayTag(GrabBusyTag))
        {
            ASC->RemoveLooseGameplayTag(GrabBusyTag);
        }

        // ── 쿨다운 부여
        if (GE_GrabCooldown)
        {
            FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
            Ctx.AddInstigator(ActorInfo->AvatarActor.Get(), nullptr);
            FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(GE_GrabCooldown, 1.f, Ctx);
            if (Spec.IsValid())
            {
                Spec.Data->DynamicGrantedTags.AddTag(MonsterTags::State_Boss_Grab_CoolTime);
                ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
            }
                
        }
    }

    // 2) BB 락 해제
    if (CachedBoss.IsValid())
    {
        if (AAIController* AI = Cast<AAIController>(CachedBoss->GetController()))
        {
            if (UBlackboardComponent* BB = AI->GetBlackboardComponent())
            {
                if (!BB_CinematicLockKey.IsNone())
                {
                    // ClearValue가 아니라 명시적으로 false 권장
                    BB->SetValueAsBool(BB_CinematicLockKey, false);
                }
             /*   if (!BB_GrabConfirmedKey.IsNone())
                {
                    BB->SetValueAsBool(BB_GrabConfirmedKey, false);
                }*/
            }
        }
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Boss_Grab::BeginApproach(AAIController* AI, ACharacter* Boss, AActor* Target)
{
    // 시선 고정(선택)
    AI->SetFocus(Target);

    // MoveTo 시작
    MoveReqId = AI->MoveToActor(Target, ApproachAcceptanceRadius, /*bStopOnOverlap=*/true);

    // 이동 완료 콜백 바인딩
    if (auto* PFC = AI->GetPathFollowingComponent())
    {
        MoveFinishedHandle = PFC->OnRequestFinished.AddUObject(this, &UGA_Boss_Grab::OnMoveFinished);
    }

    // 타임아웃: 너무 오래 걸리면 그냥 잡기 시작(프로젝트에 맞게 취향 선택)
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ApproachTimeoutHandle,
            FTimerDelegate::CreateUObject(this, &UGA_Boss_Grab::StartGrabMontage),
            MaxApproachTime, /*bLoop=*/false);
    }
}

void UGA_Boss_Grab::OnMoveFinished(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    // 다른 Move의 콜백이 들어오면 무시하고 싶다면 ID 체크
    // if (RequestID != MoveReqId) return;

    StartGrabMontage();
}

void UGA_Boss_Grab::StartGrabMontage()
{
    // 바인딩/타이머 정리
    CleanupApproachBindings();

    // 이미 끝났거나 캐릭터/몽타주 사라졌으면 방어
    if (!CachedBoss.IsValid() || !BossGrabMontage) { EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true); return; }

    // 실제 몽타주 재생 (원래 쓰던 코드 그대로)
    MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, BossGrabMontage, 1.f, StartSectionName, true, 1.f, 0.f, true);
    MontageTask->OnCompleted.AddDynamic(this, &UGA_Boss_Grab::OnMontageCompleted);
    MontageTask->OnInterrupted.AddDynamic(this, &UGA_Boss_Grab::OnMontageCancelled);
    MontageTask->OnCancelled.AddDynamic(this, &UGA_Boss_Grab::OnMontageCancelled);
    MontageTask->ReadyForActivation();
}

void UGA_Boss_Grab::CleanupApproachBindings()
{
    if (CachedBoss.IsValid())
    {
        if (AAIController* AI = Cast<AAIController>(CachedBoss->GetController()))
        {
            // 포커스 해제(선택)
            AI->ClearFocus(EAIFocusPriority::Gameplay);

            if (auto* PFC = AI->GetPathFollowingComponent())
            {
                if (MoveFinishedHandle.IsValid())
                {
                    PFC->OnRequestFinished.Remove(MoveFinishedHandle);
                    MoveFinishedHandle.Reset();
                }
            }

            // 이동 중이면 스톱(안전)
            AI->StopMovement();
        }
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ApproachTimeoutHandle);
    }
}