
#include "GAS/GA_HitReact.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Monster/MonsterCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "MonsterTags.h"
#include "TimerManager.h" 
#include "Boss/BossCharacter.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

static const FName KEY_BossState(TEXT("BossState"));
static const FName KEY_PrevBossState(TEXT("PrevBossState"));

UGA_HitReact::UGA_HitReact()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    // 정체성 태그(AssetTags)
    {
        FGameplayTagContainer Tags;
        Tags.AddTag(MonsterTags::Ability_HitReact);
        SetAssetTags(Tags);
    }

    ActivationOwnedTags.AddTag(MonsterTags::State_HitReacting);

    CancelAbilitiesWithTag.AddTag(MonsterTags::Ability_Attack);
    BlockAbilitiesWithTag.AddTag(MonsterTags::Ability_Attack);

    FAbilityTriggerData Trig;
    Trig.TriggerTag = MonsterTags::Event_Monster_Hit;
    Trig.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
    AbilityTriggers.Add(Trig);

    ActivationBlockedTags.AddTag(MonsterTags::State_Dying);
    ActivationBlockedTags.AddTag(MonsterTags::State_Dead);
    ActivationBlockedTags.AddTag(MonsterTags::State_Boss_Invuln);

    // 기본 값(헤더에도 선언 필요)
    RetryDelaySeconds = 0.02f;
    MaxHitReactDuration = 1.5f;
}

bool UGA_HitReact::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* Info, const FGameplayTagContainer*,
    const FGameplayTagContainer*, FGameplayTagContainer*) const
{
    if (!Super::CanActivateAbility(Handle, Info, nullptr, nullptr, nullptr)) return false;
    if (!Info || !Info->AbilitySystemComponent.IsValid()) return false;


    const UAbilitySystemComponent* ASC = Info->AbilitySystemComponent.Get();
    return !ASC->HasMatchingGameplayTag(MonsterTags::State_HitReacting)
        && !ASC->HasMatchingGameplayTag(MonsterTags::State_Dead);
}

UAnimMontage* UGA_HitReact::GetMonsterHitMontage(const FGameplayAbilityActorInfo* Info) const
{
    if (!Info || !Info->AvatarActor.IsValid()) return nullptr;

    const AMonsterCharacter* MC = Cast<AMonsterCharacter>(Info->AvatarActor.Get());
    if (!MC) return nullptr;

    if (UMonsterDefinition* Def = MC->GetMonsterDef())
    {
        if (!Def->HitReactMontage.IsValid())
            Def->HitReactMontage.LoadSynchronous();
        return Def->HitReactMontage.Get();
    }
    return nullptr;
}

void UGA_HitReact::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* Info, const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* EventData)
{
    UE_LOG(LogTemp, Warning, TEXT("HitReact ActivateAbility called!"));
    // 커밋 실패 시 즉시 종료
    if (!CommitAbility(Handle, Info, ActivationInfo))
    {
        EndAbility(Handle, Info, ActivationInfo, true, true);
        return;
    }
    // 상태전환
    if (Info && Info->AvatarActor.IsValid() &&
        Info->AbilitySystemComponent.IsValid() &&
        Info->AbilitySystemComponent->GetOwnerRole() == ROLE_Authority)
    {
        if (AMonsterCharacter* MC = Cast<AMonsterCharacter>(Info->AvatarActor.Get()))
        {
            MC->SetMonsterState(EMonsterState::Hit); 

            if (ABossCharacter* Boss = Cast<ABossCharacter>(MC))
            {
                AAIController* AICon = Cast<AAIController>(Boss->GetController());
                if (AICon)
                {
                    UBlackboardComponent* BB = AICon->GetBlackboardComponent();
                    if (BB)
                    {


                        // 현재 상태를 Prev로 저장
                        uint8 Cur = BB->GetValueAsEnum(KEY_BossState);
                        BB->SetValueAsEnum(KEY_PrevBossState, Cur);

                        // Hit로 전환
                        BB->SetValueAsEnum(KEY_BossState, (uint8)EBossState_BB::Hit);
                    }
                }
            }
        }
    }

    // Dead tag 감시
    if (Info && Info->AbilitySystemComponent.IsValid())
    {
        DeadTagDelegateHandle = Info->AbilitySystemComponent
            ->RegisterGameplayTagEvent(MonsterTags::State_Dead, EGameplayTagEventType::NewOrRemoved)
            .AddUObject(this, &UGA_HitReact::OnDeadTagChanged);
    }

    // 즉시 이동 정지
    if (ACharacter* C = Cast<ACharacter>(Info ? Info->AvatarActor.Get() : nullptr))
    {
        if (UCharacterMovementComponent* Move = C->GetCharacterMovement())
        {
            Move->StopMovementImmediately();
        }
    }

    // === HitResult 꺼내기 ===
    FHitResult HR;
    bool bHasHit = false;

    if (EventData && EventData->TargetData.Num() > 0)
    {
        const FGameplayAbilityTargetData* TD = EventData->TargetData.Data[0].Get();
        if (const FGameplayAbilityTargetData_SingleTargetHit* HitTD =
            static_cast<const FGameplayAbilityTargetData_SingleTargetHit*>(TD))
        {
            HR = HitTD->HitResult;
            bHasHit = true;
        }
    }
  
    if (Info && Info->AbilitySystemComponent.IsValid() &&
        Info->AbilitySystemComponent->GetOwnerRole() == ROLE_Authority)
    {
        FGameplayCueParameters Params;
        // Instigator / EffectCauser: const AActor* -> AActor* 로 변환 후 대입
        AActor* InstigatorActor = (EventData && EventData->Instigator)
            ? const_cast<AActor*>(EventData->Instigator.Get())
            : nullptr;

        // 약포인터 그대로 대입(에러 방지)
        Params.Instigator = InstigatorActor;
        Params.EffectCauser = InstigatorActor;

        AActor* TargetActor =
            (EventData && EventData->Target) ? const_cast<AActor*>(EventData->Target.Get())
            : (Info && Info->AvatarActor.IsValid() ? Info->AvatarActor.Get() : nullptr);

        if (bHasHit)
        {
            Params.SourceObject = HR.Component.Get();
            Params.Location = HR.ImpactPoint;
            Params.Normal = HR.ImpactNormal;
            Params.bReplicateLocationWhenUsingMinimalRepProxy = true;

            USceneComponent* AttachComp = HR.GetComponent();
            if (!Cast<USkeletalMeshComponent>(AttachComp))
            {
                if (TargetActor)
                {
                    if (USkeletalMeshComponent* Mesh = TargetActor->FindComponentByClass<USkeletalMeshComponent>())
                    {
                        AttachComp = Mesh;                              // ← 여기로 강제
                    }
                    else
                    {
                        AttachComp = TargetActor->GetRootComponent();   // 최후 폴백
                    }
                }
            }
            Params.TargetAttachComponent = AttachComp;
        }


        if (TargetActor)
        {
            if (UAbilitySystemComponent* TargetASC =
                UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor))
            {
                //GCN_Burst의 OnBurst가 여기서 호출됩니다.
                TargetASC->ExecuteGameplayCue(MonsterTags::GC_Impact_Hit, Params);
                

            }
        }
        else
        {
            // 폴백(그래도 보이게 하려면 시전자 ASC에 실행)
            if (Info && Info->AbilitySystemComponent.IsValid())
            {
                Info->AbilitySystemComponent->ExecuteGameplayCue(MonsterTags::GC_Impact_Hit, Params);
            }
        }
    }


    if (Info && Info->AbilitySystemComponent.IsValid() && DamageGE)
    {
        UAbilitySystemComponent* TargetASC = Info->AbilitySystemComponent.Get(); // 피격자(=몬스터)

        // 이벤트에서 공격자 ASC 얻기
        const AActor* InstigatorActor = EventData ? EventData->Instigator.Get() : nullptr;
        UAbilitySystemComponent* AttackerASC =
            InstigatorActor ? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(const_cast<AActor*>(InstigatorActor)) : nullptr;
        /*UE_LOG(LogTemp, Warning, TEXT("Target Actor: %s"), *TargetASC->GetAvatarActor()->GetName());
        UE_LOG(LogTemp, Warning, TEXT("Source Actor: %s"), *InstigatorActor->GetName());*/
        // 컨텍스트: 인스티게이터/타겟 모두 넣어주면 좋음
        FGameplayEffectContextHandle Ctx = (AttackerASC ? AttackerASC : TargetASC)->MakeEffectContext();
        if (InstigatorActor) Ctx.AddInstigator(const_cast<AActor*>(InstigatorActor), nullptr);
        if (Info && Info->AvatarActor.IsValid()) Ctx.AddSourceObject(Info->AvatarActor.Get());

        if (AttackerASC && TargetASC && TargetASC->GetOwnerRole() == ROLE_Authority)
        {
            // 스펙을 '공격자 ASC'로 만든다 (Source=플레이어)
            FGameplayEffectSpecHandle Spec = AttackerASC->MakeOutgoingSpec(DamageGE, 1.f, Ctx);
            if (Spec.IsValid())
            {
                // ByCaller 안 쓸 거면 아무 것도 넣지 말고,
                // EC의 캡처(Attack=Source, Defense=Target)에만 의존
                AttackerASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
            }
        }
    }

    //워치독 타이머(몽타주 미콜백/예외 상황 방지용)
    if (UWorld* W = GetWorld())
    {
        W->GetTimerManager().SetTimer(FailSafeHandle, this,
            &UGA_HitReact::OnFailSafeTimeout, MaxHitReactDuration, false);
    }

    // 재생 시도(재시도 로직 포함)
    TryPlayHitReactMontage(Info);
  
}

void UGA_HitReact::TryPlayHitReactMontage(const FGameplayAbilityActorInfo* Info)
{
    UAnimInstance* AnimInst = Info ? Info->AnimInstance.Get() : nullptr;
    if (!AnimInst)
    {
        USkeletalMeshComponent* Skel =
            (Info && Info->SkeletalMeshComponent.IsValid()) ? Info->SkeletalMeshComponent.Get() : nullptr;
        AnimInst = Skel ? Skel->GetAnimInstance() : nullptr;
    }

    UAnimMontage* MontageToPlay = GetMonsterHitMontage(Info);

    if (AnimInst && MontageToPlay)
    {
        UAbilityTask_PlayMontageAndWait* Task =
            UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
                this, NAME_None, MontageToPlay, 1.f, NAME_None, /*bStopWhenAbilityEnds*/ false, 0.f, 0.f, false);

        if (Task)
        {
            // 변경: BlendOut도 잡아주면 더 안전
            Task->OnBlendOut.AddDynamic(this, &UGA_HitReact::OnMontageCompleted);
            Task->OnCompleted.AddDynamic(this, &UGA_HitReact::OnMontageCompleted);
            Task->OnInterrupted.AddDynamic(this, &UGA_HitReact::OnMontageInterrupted);
            Task->OnCancelled.AddDynamic(this, &UGA_HitReact::OnMontageCancelled);
            Task->ReadyForActivation();
            return;
        }
    }

    // AnimInst/몽타주가 준비 안 되면 1틱 뒤 재시도
    if (UWorld* World = GetWorld())
    {
        FTimerDelegate RetryDel;
        RetryDel.BindUObject(this, &UGA_HitReact::OnRetryTimerElapsed);
        World->GetTimerManager().SetTimer(RetryTimerHandle, RetryDel, RetryDelaySeconds, false);
    }
    else
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
    }
}

void UGA_HitReact::OnRetryTimerElapsed()
{
    const FGameplayAbilityActorInfo* Info = CurrentActorInfo;
    if (!Info)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
        return;
    }

    UAnimInstance* AnimInst = Info ? Info->AnimInstance.Get() : nullptr;
    if (!AnimInst)
    {
        USkeletalMeshComponent* Skel =
            (Info && Info->SkeletalMeshComponent.IsValid()) ? Info->SkeletalMeshComponent.Get() : nullptr;
        AnimInst = Skel ? Skel->GetAnimInstance() : nullptr;
    }

    UAnimMontage* MontageToPlay = GetMonsterHitMontage(Info);

    if (AnimInst && MontageToPlay)
    {
        UAbilityTask_PlayMontageAndWait* Task =
            UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
                this, NAME_None, MontageToPlay, 1.f, NAME_None, false, 0.f, 0.f, false);

        if (Task)
        {
            Task->OnBlendOut.AddDynamic(this, &UGA_HitReact::OnMontageCompleted);
            Task->OnCompleted.AddDynamic(this, &UGA_HitReact::OnMontageCompleted);
            Task->OnInterrupted.AddDynamic(this, &UGA_HitReact::OnMontageInterrupted);
            Task->OnCancelled.AddDynamic(this, &UGA_HitReact::OnMontageCancelled);
            Task->ReadyForActivation();
            return;
        }
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UGA_HitReact::OnFailSafeTimeout()
{
    UE_LOG(LogTemp, Warning, TEXT("HitReact FailSafeTimeout -> Force EndAbility"));
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_HitReact::OnDeadTagChanged(const FGameplayTag Tag, int32 NewCount) // 추가: Dead 반응
{
    if (NewCount > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("HitReact cancelled due to Dead tag"));
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
    }
}

void UGA_HitReact::EndAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* Info, const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RetryTimerHandle);
        World->GetTimerManager().ClearTimer(FailSafeHandle); // 변경
    }

    if (Info && Info->AbilitySystemComponent.IsValid())
    {
        // Dead 태그 델리게이트 해제 변경
        if (DeadTagDelegateHandle.IsValid())
        {
            Info->AbilitySystemComponent
                ->RegisterGameplayTagEvent(MonsterTags::State_Dead, EGameplayTagEventType::NewOrRemoved)
                .Remove(DeadTagDelegateHandle);
            DeadTagDelegateHandle.Reset();
        }

        if (ActiveGE.IsValid())
        {
            Info->AbilitySystemComponent->RemoveActiveGameplayEffect(ActiveGE);
            ActiveGE.Invalidate();
        }
    }

    Super::EndAbility(Handle, Info, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_HitReact::OnMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}
void UGA_HitReact::OnMontageInterrupted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
void UGA_HitReact::OnMontageCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}