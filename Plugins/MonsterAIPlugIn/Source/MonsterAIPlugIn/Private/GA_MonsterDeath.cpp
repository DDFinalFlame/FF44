#include "GA_MonsterDeath.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "MonsterCharacter.h"
#include "Components/CapsuleComponent.h"
#include "MonsterTags.h" 


UGA_MonsterDeath::UGA_MonsterDeath()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    // Ability 정체성
    AbilityTags.AddTag(MonsterTags::Ability_Death);

    // 이 Ability가 켜져있는 동안 부여되는 상태
    ActivationOwnedTags.AddTag(MonsterTags::State_Dead);

    // Dead 상태면 다른 Ability는 켜지지 않게
    ActivationBlockedTags.AddTag(MonsterTags::State_Dead);

    // 트리거(순수 C++로 설정)
    FAbilityTriggerData Trig;
    Trig.TriggerTag = MonsterTags::Event_Death;
    Trig.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
    AbilityTriggers.Add(Trig);

    bServerRespectsRemoteAbilityCancellation = false;
}


// ---------- 내부 유틸 ----------

USkeletalMeshComponent* UGA_MonsterDeath::ResolveSkeletalMesh(ACharacter* Chr)
{
    if (!Chr) return nullptr;
    if (USkeletalMeshComponent* Sk = Chr->GetMesh()) return Sk;

    // 대체 탐색
    if (USkeletalMeshComponent* Any = Chr->FindComponentByClass<USkeletalMeshComponent>())
        return Any;

    TArray<USkeletalMeshComponent*> All;
    Chr->GetComponents(All);
    for (auto* C : All)
        if (C && C->GetSkinnedAsset()) return C;

    return nullptr;
}

void UGA_MonsterDeath::HardStopEverything(ACharacter* Chr, const FGameplayAbilityActorInfo* ActorInfo)
{
    // Dead 태그를 즉시 올려, 이후 Ability들이 차단되도록 (Commit 이후라 안전)
    if (UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr)
    {
        ASC->AddLooseGameplayTag(MonsterTags::State_Dead);

        // 진행 중인 다른 능력 모두 중단(자기 자신 제외)
        ASC->CancelAllAbilities(this);
    }

    // 이동/브레인/캡슐 정지
    if (Chr)
    {
        if (UCharacterMovementComponent* Move = Chr->GetCharacterMovement())
        {
            Move->StopMovementImmediately();
            Move->DisableMovement();
        }
        if (AAIController* AIC = Cast<AAIController>(Chr->GetController()))
        {
            if (AIC && AIC->BrainComponent)
            {
                AIC->BrainComponent->StopLogic(TEXT("Death"));
            }
        }
        if (UCapsuleComponent* Cap = Chr->GetCapsuleComponent())
        {
            Cap->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }

        // 재생 중인 모든 몽타주 즉시 정지
        if (USkeletalMeshComponent* Sk = ResolveSkeletalMesh(Chr))
        {
            UAnimInstance* Anim = Sk->GetAnimInstance();
            if (Anim)
            {
                Anim->StopAllMontages(0.f);
            }
        }
    }
}

// 스켈레톤을 분리시켜서 마치 몬스터가 부셔지는 효과를 내줌.
void UGA_MonsterDeath::EnterRagdoll(ACharacter* Chr)
{
    if (!Chr) return;

    if (USkeletalMeshComponent* Sk = ResolveSkeletalMesh(Chr))
    {
        Sk->SetCollisionProfileName(TEXT("Ragdoll"));
        Sk->SetAllBodiesSimulatePhysics(true);
        Sk->WakeAllRigidBodies();
        Sk->bPauseAnims = true;
        Sk->SetAllBodiesPhysicsBlendWeight(1.f);
    }
}

void UGA_MonsterDeath::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    ACharacter* Chr = Cast<ACharacter>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr);
    if (!Chr)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    //데스 시작과 동시에 모든 간섭 제거
    HardStopEverything(Chr, ActorInfo);

    UAnimMontage* MontageToPlay = nullptr;

    if (const AMonsterCharacter* MC = Cast<AMonsterCharacter>(Chr))
    {
        if (UMonsterDefinition* Def = MC->GetMonsterDef())
        {
            // SoftObjectPtr이면 필요 시 동기 로드
            if (!Def->DeathMontage.IsValid())
            {
                Def->DeathMontage.LoadSynchronous();
            }
            MontageToPlay = Def->DeathMontage.Get();
        }
    }

    bool bPlaying = false;
    if (MontageToPlay)
    {
        if (USkeletalMeshComponent* Sk = ResolveSkeletalMesh(Chr))
        {
            if (UAnimInstance* Anim = Sk->GetAnimInstance())
            {
                auto* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
                    this, NAME_None, MontageToPlay, 1.f);
                Task->OnCompleted.AddDynamic(this, &UGA_MonsterDeath::OnMontageEnded);
                Task->OnCancelled.AddDynamic(this, &UGA_MonsterDeath::OnMontageEnded);
                Task->OnInterrupted.AddDynamic(this, &UGA_MonsterDeath::OnMontageEnded);
                Task->ReadyForActivation();
                bPlaying = true;
            }
        }
    }

    // 몽타주가 없거나 재생 불가 → 즉시 래그돌 폴백
    if (!bPlaying)
    {
        EnterRagdoll(Chr);
        Chr->SetLifeSpan(5.f);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    }
}


void UGA_MonsterDeath::OnMontageEnded()
{
    const FGameplayAbilityActorInfo* Info = GetCurrentActorInfo();
    ACharacter* Chr = Info ? Cast<ACharacter>(Info->AvatarActor.Get()) : nullptr;


    if (Chr)
    {
        // 몽타주 끝난 후 래그돌 진입(연출 → 물리)
        EnterRagdoll(Chr);
        Chr->SetLifeSpan(5.f);
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_MonsterDeath::EndAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}