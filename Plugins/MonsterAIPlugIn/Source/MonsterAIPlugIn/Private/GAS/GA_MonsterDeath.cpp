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

    // [STEP2-A] 죽는 동안엔 피격/공격류를 막아 충돌 없애기
    BlockAbilitiesWithTag.AddTag(MonsterTags::Ability_HitReact);
    BlockAbilitiesWithTag.AddTag(MonsterTags::Ability_Attack);

    // Death가 활성화되면 기존 HitReact를 끊어줌
    CancelAbilitiesWithTag.AddTag(MonsterTags::Ability_HitReact);

    ActivationOwnedTags.AddTag(MonsterTags::State_Dying);
    ActivationBlockedTags.AddTag(MonsterTags::State_Dead);

    FAbilityTriggerData Trig;
    Trig.TriggerTag = MonsterTags::Event_Monster_Death;
    Trig.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
    AbilityTriggers.Add(Trig);
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
        //ASC->AddLooseGameplayTag(MonsterTags::State_Dead);

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

            if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
            {
                BB->SetValueAsBool(TEXT("IsDead"), true);                 // 즉시 Abort 유도
                BB->SetValueAsInt(TEXT("State"), (int32)EMonsterState::Dead);
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
    //    // 여기서 최종적으로 영구 Dead 부여
    //    Info->AbilitySystemComponent->AddLooseGameplayTag(MonsterTags::State_Dead);
    //    // (원하시면 무기한 GE로 GrantedTags: State.Dead 적용)
    //}


    if (Chr)
    {
        // 몽타주 끝난 후 래그돌 진입(연출 → 물리)
        NotifyBossMinionDied(Chr);
        //EnterRagdoll(Chr);
        //if (USkeletalMeshComponent* Sk = Chr->GetMesh())
        //{
        //    Sk->bPauseAnims = true;   // 마지막 프레임 고정
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
        BossActor = MC->GetOwnerBoss();  // 소환 시 주입한 보스
    }

    if (BossActor)
    {
        FGameplayEventData Data;
        Data.EventTag = MonsterTags::Event_Minion_Died;
        Data.Instigator = DeadChr;      // 누가 죽었는지
        Data.Target = BossActor;    // 보스

        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
            BossActor, MonsterTags::Event_Minion_Died, Data);

        bSentBossNotify = true;
    }
}

void UGA_MonsterDeath::TrySpawnDrop(ACharacter* DeadChr)
{
    if (bDropSpawned) return;
    if (!DeadChr) return;

    // 서버에서만
    const UAbilitySystemComponent* ASC =
        GetCurrentActorInfo() ? GetCurrentActorInfo()->AbilitySystemComponent.Get() : nullptr;
    if (!ASC || ASC->GetOwnerRole() != ROLE_Authority) return;

    if (!DropActorClass) return;
    if (FMath::FRand() > DropChance) return; // 확률 미스

    UWorld* World = DeadChr->GetWorld();
    if (!World) return;

    FVector SpawnLoc = DeadChr->GetActorLocation();
    FRotator SpawnRot = FRotator::ZeroRotator;

    // 바닥 정렬(선택)
    if (bDropAlignToGround)
    {
        FHitResult Hit;
        const FVector Start = SpawnLoc + FVector(0, 0, 50.f);
        const FVector End = SpawnLoc + FVector(0, 0, -2000.f);
        FCollisionQueryParams Params(SCENE_QUERY_STAT(DeathDropTrace), false, DeadChr);
        if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
        {
            SpawnLoc = Hit.ImpactPoint;
            SpawnRot = Hit.ImpactNormal.Rotation(); // 노멀 기준 회전
        }
    }

    SpawnLoc.Z += DropSpawnZOffset;

    FActorSpawnParameters S;
    S.Owner = DeadChr;
    S.Instigator = DeadChr;
    S.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // 필요 시 지연 스폰(초기 세팅 필요하면)
    AActor* Drop = World->SpawnActor<AActor>(DropActorClass, SpawnLoc, SpawnRot, S);
    if (Drop)
    {
        bDropSpawned = true;
        // 필요하면 인터페이스/초기화 함수 호출
        // IYourDropInterface::Execute_Init(Drop, …);
    }
}