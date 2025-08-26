#include "GA_MonsterAssemble.h"
#include "MonsterTags.h"

#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "MonsterCharacter.h"

static bool HasPhysicsBody(USkeletalMeshComponent* Sk, const FName& Bone)
{
    if (!Sk) return false;
    UPhysicsAsset* PA = Sk->GetPhysicsAsset();
    if (!PA) return false;
    return PA->FindBodyIndex(Bone) != INDEX_NONE;
}

UGA_MonsterAssemble::UGA_MonsterAssemble()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    AbilityTags.AddTag(MonsterTags::Ability_Assemble);
    ActivationOwnedTags.AddTag(MonsterTags::State_Assembling);

    FAbilityTriggerData Trig;
    Trig.TriggerTag = MonsterTags::Event_Assemble;
    Trig.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
    AbilityTriggers.Add(Trig);
}

void UGA_MonsterAssemble::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo ActInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, Info, ActInfo))
    {
        EndAbility(Handle, Info, ActInfo, true, true);
        return;
    }

    ACharacter* Chr = Info ? Cast<ACharacter>(Info->AvatarActor.Get()) : nullptr;
    if (!Chr)
    {
        EndAbility(Handle, Info, ActInfo, true, true);
        return;
    }

    OwnerChar = Chr;
    Recover_Start(Chr); // 물리 끄지 않고, 정렬 후 바로 AssembleStep()으로
}

void UGA_MonsterAssemble::EndAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* Info,
    const FGameplayAbilityActivationInfo ActInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    ACharacter* Chr = Info ? Cast<ACharacter>(Info->AvatarActor.Get()) : nullptr;
    if (Chr)
    {
        Chr->GetWorldTimerManager().ClearTimer(TH_BlendTick);
    }

    Super::EndAbility(Handle, Info, ActInfo, bReplicateEndAbility, bWasCancelled);
}

USkeletalMeshComponent* UGA_MonsterAssemble::GetMesh(ACharacter* Chr) const
{
    if (!Chr) return nullptr;
    USkeletalMeshComponent* Sk = Chr->GetMesh();
    if (Sk) return Sk;

    TArray<USkeletalMeshComponent*> All;
    Chr->GetComponents(All);
    for (int i = 0; i < All.Num(); ++i)
    {
        if (All[i] && All[i]->GetSkinnedAsset()) return All[i];
    }
    return nullptr;
}

UCapsuleComponent* UGA_MonsterAssemble::GetCapsule(ACharacter* Chr) const
{
    return Chr ? Chr->GetCapsuleComponent() : nullptr;
}

UCharacterMovementComponent* UGA_MonsterAssemble::GetMove(ACharacter* Chr) const
{
    return Chr ? Chr->GetCharacterMovement() : nullptr;
}

AAIController* UGA_MonsterAssemble::GetAI(ACharacter* Chr) const
{
    return Chr ? Cast<AAIController>(Chr->GetController()) : nullptr;
}

FHitResult UGA_MonsterAssemble::TraceFloor(ACharacter* Chr, const FVector& From, float DownDist, ECollisionChannel Ch)
{
    FHitResult Hit;
    if (!Chr) return Hit;

    FCollisionQueryParams QP(SCENE_QUERY_STAT(Assemble_Floor), false, Chr);
    FVector Start = From + FVector(0, 0, 50.f);
    FVector End = From - FVector(0, 0, DownDist);

    Chr->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, Ch, QP);
    return Hit;
}

void UGA_MonsterAssemble::Recover_Start(ACharacter* Chr)
{
    USkeletalMeshComponent* Sk = GetMesh(Chr);
    UCapsuleComponent* Cap = GetCapsule(Chr);
    UCharacterMovementComponent* Mv = GetMove(Chr);
    if (!Sk || !Cap || !Mv)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    // 0) 애님 켜두기(Idle로 수렴)
    Sk->bPauseAnims = false;

    // 1) AI 잠시 정지
    if (AAIController* AIC = GetAI(Chr))
        if (AIC->BrainComponent) AIC->BrainComponent->StopLogic(TEXT("Assemble Recover"));

    // 2) 충돌: 메시는 약화, 캡슐은 활성
    Sk->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
    Cap->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // 3) 목표 Yaw (Pelvis 기준으로 추정)
    float DesiredYaw = Chr->GetActorRotation().Yaw;
    {
        int PelvisIdx = Sk->GetBoneIndex(PelvisBone);
        if (PelvisIdx != INDEX_NONE)
        {
            FRotator PelvisRot = Sk->GetBoneQuaternion(PelvisBone).Rotator();
            DesiredYaw = PelvisRot.Yaw;
        }
    }

    // 4) 바닥 Z 보정
    FVector From = Sk->GetComponentLocation();
    FHitResult FloorHit = TraceFloor(Chr, From, FloorTraceDist, FloorTraceChannel);

    float Half = Cap->GetScaledCapsuleHalfHeight();
    FVector DesiredLoc = Chr->GetActorLocation();
    if (FloorHit.IsValidBlockingHit())
        DesiredLoc.Z = FloorHit.ImpactPoint.Z + Half + StandSnapUpOffset;

    // 5) 텔레포트(겹침 무시)
    Chr->TeleportTo(DesiredLoc, FRotator(0.f, DesiredYaw, 0.f), false, true);

    // 6) 메시를 캡슐에 재부착 + 초기 상대값 복구
    /*Sk->AttachToComponent(Cap, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    if (AMonsterCharacter* MC = Cast<AMonsterCharacter>(Chr))
    {
        Sk->SetRelativeLocation(MC->MeshInitRelLoc, false, nullptr, ETeleportType::TeleportPhysics);
        Sk->SetRelativeRotation(MC->MeshInitRelRot, false, nullptr, ETeleportType::TeleportPhysics);
        Sk->SetRelativeScale3D(MC->MeshInitRelScale);
    }*/

    // 7) 전신 물리/블렌드는 "살려둠" (체인에서 하나씩 끌 예정)
    //    혹시 블렌드가 0으로 내려가 있다면 1로 다시 올려둡니다.
    Sk->SetAllBodiesPhysicsBlendWeight(1.f);

    Sk->SetAllBodiesSimulatePhysics(true);
    Sk->WakeAllRigidBodies();

    // 8) 이동/회전 규칙: 바로 뒤집히지 않도록 잠깐 고정
    Mv->StopMovementImmediately();
    Mv->SetMovementMode(MOVE_Walking);
    Mv->bUseControllerDesiredRotation = false;
    Mv->bOrientRotationToMovement = false;

    // 9) 순차 조립 시작
    CurrentChainIndex = 0;
    AssembleStep();
}

void UGA_MonsterAssemble::AssembleStep()
{
    ACharacter* Chr = OwnerChar.Get();
    if (!Chr) return;

    if (CurrentChainIndex >= AssembleOrder.Num())
    {
        // 모든 본 조립 끝: 안전 정렬 후 AI 재가동
        StandUpFix(Chr);
        if (AAIController* AIC = GetAI(Chr))
            if (AIC->BrainComponent) AIC->BrainComponent->StartLogic();

        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }
    USkeletalMeshComponent* Sk = GetMesh(Chr);
    if (!Sk) return;

    // 바디가 있는 본만 진행. 없으면 다음 본으로 스킵
    while (CurrentChainIndex < AssembleOrder.Num())
    {
        const FName Bone = AssembleOrder[CurrentChainIndex];
        if (HasPhysicsBody(Sk, Bone))
        {
            CurrentBone = Bone;
            StartSmoothBlend(CurrentBone, StepBlendTime);
            return;
        }
        ++CurrentChainIndex; // 바디 없으면 스킵
    }

    // 모두 스킵됐다면 즉시 마무리
    StandUpFix(Chr);
    if (AAIController* AIC = GetAI(Chr))
        if (AIC->BrainComponent) AIC->BrainComponent->StartLogic();
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_MonsterAssemble::StartSmoothBlend(const FName& StartBone, float Duration)
{
    ACharacter* Chr = OwnerChar.Get();
    if (!Chr) return;
    USkeletalMeshComponent* Sk = GetMesh(Chr);
    if (!Sk) return;

    // 바디 없는 본이 들어오면 바로 다음 체인으로
    if (!HasPhysicsBody(Sk, StartBone))
    {
        CurrentChainIndex++;
        AssembleStep();
        return;
    }

    CurrentBone = StartBone;
    BlendElapsed = 0.f;
    BlendDuration = FMath::Max(0.1f, Duration);

    // 시작은 물리 100%
    Sk->SetAllBodiesBelowPhysicsBlendWeight(StartBone, 1.0f, false, true);

    Chr->GetWorldTimerManager().ClearTimer(TH_BlendTick);
    Chr->GetWorldTimerManager().SetTimer(
        TH_BlendTick, this, &UGA_MonsterAssemble::TickSmoothBlend, TickInterval, true, 0.f);
}

float UGA_MonsterAssemble::EvalBlendAlpha(float T01)
{
    float t = FMath::Clamp(T01, 0.f, 1.f);
    return t * t * (3.f - 2.f * t); // S-curve
}

void UGA_MonsterAssemble::TickSmoothBlend()
{
    ACharacter* Chr = OwnerChar.Get();
    if (!Chr) return;
    USkeletalMeshComponent* Sk = GetMesh(Chr);
    if (!Sk) return;

    BlendElapsed += TickInterval;
    float T = FMath::Clamp(BlendElapsed / BlendDuration, 0.f, 1.f);
    float Alpha = EvalBlendAlpha(T);      // 애님 비중
    float Weight = 1.f - Alpha;            // 물리 비중

    Sk->SetAllBodiesBelowPhysicsBlendWeight(CurrentBone, Weight, false, true);

    if (T >= 1.f)
    {
        // 체인 완료: 애님 100%, 해당 체인 물리 OFF
        Sk->SetAllBodiesBelowPhysicsBlendWeight(CurrentBone, 0.f, false, true);
        Sk->SetAllBodiesBelowSimulatePhysics(CurrentBone, false, true);

        Chr->GetWorldTimerManager().ClearTimer(TH_BlendTick);
        CurrentChainIndex++;
        AssembleStep(); // 다음 본 진행
    }
}

void UGA_MonsterAssemble::StandUpFix(ACharacter* Chr)
{
    if (!Chr) return;

    USkeletalMeshComponent* Sk = GetMesh(Chr);
    UCapsuleComponent* Cap = GetCapsule(Chr);
    UCharacterMovementComponent* Mv = GetMove(Chr);
    if (!Sk || !Cap || !Mv) return;

    // 1) 물리 완전 OFF + 블렌드 0 + 애님 ON
    Sk->SetAllBodiesPhysicsBlendWeight(0.f);
    Sk->SetAllBodiesSimulatePhysics(false);
    Sk->bPauseAnims = false;

    // 2) 충돌 정상화
    Sk->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 또는 CharacterMesh
    Cap->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // 3) 메시-캡슐 재부착 + 초기 상대값 복구
    Sk->AttachToComponent(Cap, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    if (AMonsterCharacter* MC = Cast<AMonsterCharacter>(Chr))
    {
        Sk->SetRelativeLocation(MC->MeshInitRelLoc, false, nullptr, ETeleportType::TeleportPhysics);
        Sk->SetRelativeRotation(MC->MeshInitRelRot, false, nullptr, ETeleportType::TeleportPhysics);
        Sk->SetRelativeScale3D(MC->MeshInitRelScale);
    }

    // 4) 바닥 스냅 (Pelvis/Spine Yaw 사용)
    float DesiredYaw = Chr->GetActorRotation().Yaw;
    {
        int PelvisIdx = Sk->GetBoneIndex(PelvisBone);
        if (PelvisIdx != INDEX_NONE)
        {
            FRotator PelvisRot = Sk->GetBoneQuaternion(PelvisBone).Rotator();
            DesiredYaw = PelvisRot.Yaw;
        }
    }

    FHitResult FloorHit = TraceFloor(Chr, Chr->GetActorLocation(), FloorTraceDist, FloorTraceChannel);
    float Half = Cap->GetScaledCapsuleHalfHeight();
    FVector DesiredLoc = Chr->GetActorLocation();
    if (FloorHit.IsValidBlockingHit())
        DesiredLoc.Z = FloorHit.ImpactPoint.Z + Half + StandSnapUpOffset;

    Chr->TeleportTo(DesiredLoc, FRotator(0.f, DesiredYaw, 0.f), false, true);

    // 5) 관성/다이나믹스 리셋
    Sk->SetAllPhysicsLinearVelocity(FVector::ZeroVector, false);
    Sk->SetAllPhysicsAngularVelocityInDegrees(FVector::ZeroVector, false);
    Sk->ResetAnimInstanceDynamics(ETeleportType::TeleportPhysics);
    Sk->ForceClothNextUpdateTeleportAndReset();
    Sk->RefreshBoneTransforms();

    // 6) 이동/회전 규칙 복귀 + 오버랩 갱신
    RestoreRotateFlags(Chr);
    Cap->UpdateOverlaps();
}

void UGA_MonsterAssemble::RestoreRotateFlags(ACharacter* Chr)
{
    UCharacterMovementComponent* Mv = GetMove(Chr);
    if (!Mv) return;

    // 평소 선호값으로 복귀(상황에 맞게 바꾸세요)
    Mv->bUseControllerDesiredRotation = false;
    Mv->bOrientRotationToMovement = true;
}