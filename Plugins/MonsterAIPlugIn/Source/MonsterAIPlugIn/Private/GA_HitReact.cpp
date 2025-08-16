#include "GA_HitReact.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MonsterCharacter.h"
#include "Components/SkeletalMeshComponent.h"

static FGameplayTag TAG_Ability_HitReact() { return FGameplayTag::RequestGameplayTag(TEXT("Ability.Monster.HitReact")); }
static FGameplayTag TAG_Ability_Attack() { return FGameplayTag::RequestGameplayTag(TEXT("Ability.Monster.Attack")); }
static FGameplayTag TAG_State_HitReacting() { return FGameplayTag::RequestGameplayTag(TEXT("Ability.State.HitReacting")); }
static FGameplayTag TAG_Event_Hit() { return FGameplayTag::RequestGameplayTag(TEXT("Event.Hit")); }

UGA_HitReact::UGA_HitReact()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    //5.6 방식: SetAssetTags 사용
    {
        FGameplayTagContainer Tags;
        Tags.AddTag(TAG_Ability_HitReact());
        SetAssetTags(Tags);
    }

    ActivationOwnedTags.AddTag(TAG_State_HitReacting());

    CancelAbilitiesWithTag.AddTag(TAG_Ability_Attack());
    BlockAbilitiesWithTag.AddTag(TAG_Ability_Attack());

    FAbilityTriggerData Trig;
    Trig.TriggerTag = TAG_Event_Hit();
    Trig.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
    AbilityTriggers.Add(Trig);
}

bool UGA_HitReact::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* Info, const FGameplayTagContainer*,
    const FGameplayTagContainer*, FGameplayTagContainer*) const
{
    if (!Super::CanActivateAbility(Handle, Info, nullptr, nullptr, nullptr)) return false;
    if (!Info || !Info->AbilitySystemComponent.IsValid()) return false;

    // 이미 피격중이면 금지
    return !Info->AbilitySystemComponent->HasMatchingGameplayTag(TAG_State_HitReacting());
}

UAnimMontage* UGA_HitReact::GetMonsterHitMontage(const FGameplayAbilityActorInfo* Info) const
{
    if (!Info || !Info->AvatarActor.IsValid()) return nullptr;

    const AMonsterCharacter* MC = Cast<AMonsterCharacter>(Info->AvatarActor.Get());
    if (!MC) return nullptr;

    if (UMonsterDefinition* Def = MC->GetMonsterDef())
    {
        // 필요 시 동기 로드
        if (!Def->HitReactMontage.IsValid()) Def->HitReactMontage.LoadSynchronous();
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

    UAnimInstance* AnimInst = Info ? Info->AnimInstance.Get() : nullptr;
    if (!AnimInst)
    {
        USkeletalMeshComponent* Skel =
            (Info && Info->SkeletalMeshComponent.IsValid()) ? Info->SkeletalMeshComponent.Get() : nullptr;
        AnimInst = Skel ? Skel->GetAnimInstance() : nullptr;
    }

    // 아직 없으면 1틱 뒤 재시도
    if (!AnimInst)
    {
        FTimerHandle Tmp;
        GetWorld()->GetTimerManager().SetTimer(Tmp, [this, Handle, Info, ActivationInfo]()
            {
                // 내부 재생
                UAnimMontage* MontageToPlay = GetMonsterHitMontage(Info);
                UAnimInstance* AnimInst2 = nullptr;
                if (Info)
                {
                    AnimInst2 = Info->AnimInstance.Get();
                    if (!AnimInst2)
                    {
                        USkeletalMeshComponent* Skel =
                            (Info && Info->SkeletalMeshComponent.IsValid()) ? Info->SkeletalMeshComponent.Get() : nullptr;
                        AnimInst2 = Skel ? Skel->GetAnimInstance() : nullptr;
                    }
                }

                if (MontageToPlay && AnimInst2)
                {
                    auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
                        this, NAME_None, MontageToPlay, 1.f, NAME_None, false, 0.f, 0.f, false);
                    Task->OnCompleted.AddDynamic(this, &UGA_HitReact::OnMontageCompleted);
                    Task->OnInterrupted.AddDynamic(this, &UGA_HitReact::OnMontageInterrupted);
                    Task->OnCancelled.AddDynamic(this, &UGA_HitReact::OnMontageCancelled);
                    Task->ReadyForActivation();
                }
                else
                {
                    EndAbility(Handle, Info, ActivationInfo, false, false);
                }
            }, 0.02f, false);

        return;
    }

    // 즉시 재생 경로
    UAnimMontage* MontageToPlay = GetMonsterHitMontage(Info);
    if (MontageToPlay)
    {
        auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, NAME_None, MontageToPlay, 1.f, NAME_None, false, 0.f, 0.f, false);
        Task->OnCompleted.AddDynamic(this, &UGA_HitReact::OnMontageCompleted);
        Task->OnInterrupted.AddDynamic(this, &UGA_HitReact::OnMontageInterrupted);
        Task->OnCancelled.AddDynamic(this, &UGA_HitReact::OnMontageCancelled);
        Task->ReadyForActivation();
        return;
    }

    // 몽타주가 없으면(또는 Task 실패) 즉시 종료
    EndAbility(Handle, Info, ActivationInfo, false, false);
}


void UGA_HitReact::EndAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* Info, const FGameplayAbilityActivationInfo ActivationInfo,
    bool Replicate, bool WasCancelled)
{
    if (Info && Info->AbilitySystemComponent.IsValid() && ActiveGE.IsValid())
    {
        Info->AbilitySystemComponent->RemoveActiveGameplayEffect(ActiveGE);
    }
    Super::EndAbility(Handle, Info, ActivationInfo, Replicate, WasCancelled);
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