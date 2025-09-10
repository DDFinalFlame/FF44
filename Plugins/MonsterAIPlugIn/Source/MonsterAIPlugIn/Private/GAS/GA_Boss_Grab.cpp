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

    // Commit(��ٿ�/���/�±� ��)
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    ACharacter* Boss = Cast<ACharacter>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr);
    ACharacter* Victim = nullptr;

    if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
        if (GrabBusyTag.IsValid()) ASC->AddLooseGameplayTag(GrabBusyTag);

    // BT �����嵵 true�� (���ڷ����Ϳ��� ����)
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

    // 1) (����) BB�� Victim ��� �� ANS�� BB�� Victim�� ȹ���� �� �ְ�
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

  //   2) (����) Motion Warping Ÿ�� ����
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
                return; // Move �Ϸ�/Ÿ�Ӿƿ� �ݹ鿡�� StartGrabMontage ȣ��
            }
        }
    }

    // �Ÿ� ����ϸ� ��ٷ� ��Ÿ�� ����
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
    // ���� Ÿ�̸�/���ε� ���� �־��ٸ� ȣ��
    CleanupApproachBindings();



    // 1) �±� ����
    if (UAbilitySystemComponent* ASC = CurrentActorInfo ? CurrentActorInfo->AbilitySystemComponent.Get() : nullptr)
    {
        if (GrabBusyTag.IsValid() && ASC->HasMatchingGameplayTag(GrabBusyTag))
        {
            ASC->RemoveLooseGameplayTag(GrabBusyTag);
        }

        // ���� ��ٿ� �ο�
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

    // 2) BB �� ����
    if (CachedBoss.IsValid())
    {
        if (AAIController* AI = Cast<AAIController>(CachedBoss->GetController()))
        {
            if (UBlackboardComponent* BB = AI->GetBlackboardComponent())
            {
                if (!BB_CinematicLockKey.IsNone())
                {
                    // ClearValue�� �ƴ϶� ��������� false ����
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
    // �ü� ����(����)
    AI->SetFocus(Target);

    // MoveTo ����
    MoveReqId = AI->MoveToActor(Target, ApproachAcceptanceRadius, /*bStopOnOverlap=*/true);

    // �̵� �Ϸ� �ݹ� ���ε�
    if (auto* PFC = AI->GetPathFollowingComponent())
    {
        MoveFinishedHandle = PFC->OnRequestFinished.AddUObject(this, &UGA_Boss_Grab::OnMoveFinished);
    }

    // Ÿ�Ӿƿ�: �ʹ� ���� �ɸ��� �׳� ��� ����(������Ʈ�� �°� ���� ����)
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
    // �ٸ� Move�� �ݹ��� ������ �����ϰ� �ʹٸ� ID üũ
    // if (RequestID != MoveReqId) return;

    StartGrabMontage();
}

void UGA_Boss_Grab::StartGrabMontage()
{
    // ���ε�/Ÿ�̸� ����
    CleanupApproachBindings();

    // �̹� �����ų� ĳ����/��Ÿ�� ��������� ���
    if (!CachedBoss.IsValid() || !BossGrabMontage) { EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true); return; }

    // ���� ��Ÿ�� ��� (���� ���� �ڵ� �״��)
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
            // ��Ŀ�� ����(����)
            AI->ClearFocus(EAIFocusPriority::Gameplay);

            if (auto* PFC = AI->GetPathFollowingComponent())
            {
                if (MoveFinishedHandle.IsValid())
                {
                    PFC->OnRequestFinished.Remove(MoveFinishedHandle);
                    MoveFinishedHandle.Reset();
                }
            }

            // �̵� ���̸� ����(����)
            AI->StopMovement();
        }
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ApproachTimeoutHandle);
    }
}