#include "GAS/GA_MonsterDeath.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "Monster/MonsterCharacter.h"
#include "Components/CapsuleComponent.h"
#include "MonsterTags.h" 
#include "BehaviorTree/BlackboardComponent.h"


UGA_MonsterDeath::UGA_MonsterDeath()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    AbilityTags.AddTag(MonsterTags::Ability_Death);

    // [STEP2-A] �״� ���ȿ� �ǰ�/���ݷ��� ���� �浹 ���ֱ�
    BlockAbilitiesWithTag.AddTag(MonsterTags::Ability_HitReact);
    BlockAbilitiesWithTag.AddTag(MonsterTags::Ability_Attack);

    // Death�� Ȱ��ȭ�Ǹ� ���� HitReact�� ������
    CancelAbilitiesWithTag.AddTag(MonsterTags::Ability_HitReact);

    ActivationOwnedTags.AddTag(MonsterTags::State_Dying);
    ActivationBlockedTags.AddTag(MonsterTags::State_Dead);

    FAbilityTriggerData Trig;
    Trig.TriggerTag = MonsterTags::Event_Monster_Death;
    Trig.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
    AbilityTriggers.Add(Trig);
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
        //ASC->AddLooseGameplayTag(MonsterTags::State_Dead);

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

            if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
            {
                BB->SetValueAsBool(TEXT("IsDead"), true);                 // ��� Abort ����
                BB->SetValueAsInt(TEXT("State"), (int32)EMonsterState::Dead);
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
        TrySpawnDrop(Chr);
        Chr->SetLifeSpan(5.f);

        NotifyBossMinionDied(Chr);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    }
}


void UGA_MonsterDeath::OnMontageEnded()
{
    const FGameplayAbilityActorInfo* Info = GetCurrentActorInfo();
    ACharacter* Chr = Info ? Cast<ACharacter>(Info->AvatarActor.Get()) : nullptr;

    //if (Info && Info->AbilitySystemComponent.IsValid())
    //{
    //    // ���⼭ ���������� ���� Dead �ο�
    //    Info->AbilitySystemComponent->AddLooseGameplayTag(MonsterTags::State_Dead);
    //    // (���Ͻø� ������ GE�� GrantedTags: State.Dead ����)
    //}


    if (Chr)
    {
        // ��Ÿ�� ���� �� ���׵� ����(���� �� ����)
        NotifyBossMinionDied(Chr);
        //EnterRagdoll(Chr);
        //if (USkeletalMeshComponent* Sk = Chr->GetMesh())
        //{
        //    Sk->bPauseAnims = true;   // ������ ������ ����
        //}
        //Chr->SetLifeSpan(15.f);
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


void UGA_MonsterDeath::NotifyBossMinionDied(ACharacter* DeadChr)
{
    if (bSentBossNotify || !DeadChr) return;

    AActor* BossActor = nullptr;

    if (const AMonsterCharacter* MC = Cast<AMonsterCharacter>(DeadChr))
    {
        BossActor = MC->GetOwnerBoss();  // ��ȯ �� ������ ����
    }

    if (BossActor)
    {
        FGameplayEventData Data;
        Data.EventTag = MonsterTags::Event_Minion_Died;
        Data.Instigator = DeadChr;      // ���� �׾�����
        Data.Target = BossActor;    // ����

        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
            BossActor, MonsterTags::Event_Minion_Died, Data);

        bSentBossNotify = true;
    }
}

void UGA_MonsterDeath::TrySpawnDrop(ACharacter* DeadChr)
{
    if (bDropSpawned) return;
    if (!DeadChr) return;

    // ����������
    const UAbilitySystemComponent* ASC =
        GetCurrentActorInfo() ? GetCurrentActorInfo()->AbilitySystemComponent.Get() : nullptr;
    if (!ASC || ASC->GetOwnerRole() != ROLE_Authority) return;

    if (!DropActorClass) return;
    if (FMath::FRand() > DropChance) return; // Ȯ�� �̽�

    UWorld* World = DeadChr->GetWorld();
    if (!World) return;

    FVector SpawnLoc = DeadChr->GetActorLocation();
    FRotator SpawnRot = FRotator::ZeroRotator;

    // �ٴ� ����(����)
    if (bDropAlignToGround)
    {
        FHitResult Hit;
        const FVector Start = SpawnLoc + FVector(0, 0, 50.f);
        const FVector End = SpawnLoc + FVector(0, 0, -2000.f);
        FCollisionQueryParams Params(SCENE_QUERY_STAT(DeathDropTrace), false, DeadChr);
        if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
        {
            SpawnLoc = Hit.ImpactPoint;
            SpawnRot = Hit.ImpactNormal.Rotation(); // ��� ���� ȸ��
        }
    }

    SpawnLoc.Z += DropSpawnZOffset;

    FActorSpawnParameters S;
    S.Owner = DeadChr;
    S.Instigator = DeadChr;
    S.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // �ʿ� �� ���� ����(�ʱ� ���� �ʿ��ϸ�)
    AActor* Drop = World->SpawnActor<AActor>(DropActorClass, SpawnLoc, SpawnRot, S);
    if (Drop)
    {
        bDropSpawned = true;
        // �ʿ��ϸ� �������̽�/�ʱ�ȭ �Լ� ȣ��
        // IYourDropInterface::Execute_Init(Drop, ��);
    }
}