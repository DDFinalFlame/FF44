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

    // Ability ��ü��
    AbilityTags.AddTag(MonsterTags::Ability_Death);

    // �� Ability�� �����ִ� ���� �ο��Ǵ� ����
    ActivationOwnedTags.AddTag(MonsterTags::State_Dead);

    // Dead ���¸� �ٸ� Ability�� ������ �ʰ�
    ActivationBlockedTags.AddTag(MonsterTags::State_Dead);

    // Ʈ����(���� C++�� ����)
    FAbilityTriggerData Trig;
    Trig.TriggerTag = MonsterTags::Event_Death;
    Trig.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
    AbilityTriggers.Add(Trig);

    bServerRespectsRemoteAbilityCancellation = false;
}


// ---------- ���� ��ƿ ----------

USkeletalMeshComponent* UGA_MonsterDeath::ResolveSkeletalMesh(ACharacter* Chr)
{
    if (!Chr) return nullptr;
    if (USkeletalMeshComponent* Sk = Chr->GetMesh()) return Sk;

    // ��ü Ž��
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
    // Dead �±׸� ��� �÷�, ���� Ability���� ���ܵǵ��� (Commit ���Ķ� ����)
    if (UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr)
    {
        ASC->AddLooseGameplayTag(MonsterTags::State_Dead);

        // ���� ���� �ٸ� �ɷ� ��� �ߴ�(�ڱ� �ڽ� ����)
        ASC->CancelAllAbilities(this);
    }

    // �̵�/�극��/ĸ�� ����
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

        // ��� ���� ��� ��Ÿ�� ��� ����
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

// ���̷����� �и����Ѽ� ��ġ ���Ͱ� �μ����� ȿ���� ����.
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

    //���� ���۰� ���ÿ� ��� ���� ����
    HardStopEverything(Chr, ActorInfo);

    UAnimMontage* MontageToPlay = nullptr;

    if (const AMonsterCharacter* MC = Cast<AMonsterCharacter>(Chr))
    {
        if (UMonsterDefinition* Def = MC->GetMonsterDef())
        {
            // SoftObjectPtr�̸� �ʿ� �� ���� �ε�
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

    // ��Ÿ�ְ� ���ų� ��� �Ұ� �� ��� ���׵� ����
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
        // ��Ÿ�� ���� �� ���׵� ����(���� �� ����)
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