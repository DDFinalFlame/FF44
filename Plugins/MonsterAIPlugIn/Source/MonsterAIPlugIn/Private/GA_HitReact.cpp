#include "GA_HitReact.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MonsterCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "MonsterTags.h"
#include "TimerManager.h" 

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
    Trig.TriggerTag = MonsterTags::Event_Hit;
    Trig.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
    AbilityTriggers.Add(Trig);

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
    const FGameplayEventData*)
{
    UE_LOG(LogTemp, Warning, TEXT("HitReact ActivateAbility called!"));

    if (!CommitAbility(Handle, Info, ActivationInfo))
    {
        EndAbility(Handle, Info, ActivationInfo, true, true);
        return;
    }

    if (Info && Info->AbilitySystemComponent.IsValid())
    {
        DeadTagDelegateHandle = Info->AbilitySystemComponent
            ->RegisterGameplayTagEvent(MonsterTags::State_Dead, EGameplayTagEventType::NewOrRemoved)
            .AddUObject(this, &UGA_HitReact::OnDeadTagChanged);
    }

    if (ACharacter* C = Cast<ACharacter>(Info ? Info->AvatarActor.Get() : nullptr))
    {
        if (UCharacterMovementComponent* Move = C->GetCharacterMovement())
        {
            Move->StopMovementImmediately();
        }
    }

    // 상태 GE 부여(있으면)
    if (HitReactEffectClass && Info && Info->AbilitySystemComponent.IsValid())
    {
        ActiveGE = Info->AbilitySystemComponent->ApplyGameplayEffectToSelf(
            HitReactEffectClass->GetDefaultObject<UGameplayEffect>(), 1.f,
            Info->AbilitySystemComponent->MakeEffectContext());
    }


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
            // ★ 변경: BlendOut도 잡아주면 더 안전
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