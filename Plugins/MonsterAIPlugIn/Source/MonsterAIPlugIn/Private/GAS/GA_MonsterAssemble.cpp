#include "GAS/GA_MonsterAssemble.h"
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
#include "Monster/MonsterCharacter.h"
#include "NiagaraFunctionLibrary.h"  
#include "Kismet/GameplayStatics.h"  
#include "NiagaraComponent.h"


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

    {
        FGameplayTagContainer AssetTags;
        AssetTags.AddTag(MonsterTags::Ability_Assemble);
        SetAssetTags(AssetTags);
    }

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
    StopAssembleFX();
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
    //Chr->TeleportTo(DesiredLoc, FRotator(0.f, DesiredYaw, 0.f), false, true);

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
    StartAssembleFX(Chr);          
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
        BeginGetUp(Chr);
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

    const float PosThresh = 1.0f;  // 위치 허용 오차(cm)
    const float YawThresh = 1.0f;  // 회전 허용 오차(도)

    const FVector CurLoc = Chr->GetActorLocation();
    const FRotator CurRot = Chr->GetActorRotation();

    const bool bNeedPosSnap = (FVector::DistSquared(CurLoc, DesiredLoc) > PosThresh * PosThresh);
    const bool bNeedYawSnap = (FMath::Abs(FMath::FindDeltaAngleDegrees(CurRot.Yaw, DesiredYaw)) > YawThresh);

    if (bNeedPosSnap || bNeedYawSnap)
    {
        // 오차가 정말 클 때만 스냅
        Chr->TeleportTo(DesiredLoc, FRotator(0.f, DesiredYaw, 0.f), false, true);
    }
    else
    {
        // 보통은 부드럽게 세우기(한 프레임 마무리)
        Chr->SetActorLocation(DesiredLoc, /*bSweep=*/false, nullptr, ETeleportType::None);
        Chr->SetActorRotation(FRotator(0.f, DesiredYaw, 0.f), ETeleportType::None);
    }

    // 5) 관성/다이나믹스 리셋
    Sk->SetAllPhysicsLinearVelocity(FVector::ZeroVector, false);
    Sk->SetAllPhysicsAngularVelocityInDegrees(FVector::ZeroVector, false);
    Sk->ResetAnimInstanceDynamics(ETeleportType::TeleportPhysics);
    Sk->ForceClothNextUpdateTeleportAndReset();
    Sk->RefreshBoneTransforms();

    // 6) 이동/회전 규칙 복귀 + 오버랩 갱신
    RestoreRotateFlags(Chr);
    Cap->UpdateOverlaps();

    if (AMonsterCharacter* MC = Cast<AMonsterCharacter>(Chr))
    {
        MC->SetMonsterState(EMonsterState::CombatReady);
    }
}

void UGA_MonsterAssemble::RestoreRotateFlags(ACharacter* Chr)
{
    UCharacterMovementComponent* Mv = GetMove(Chr);
    if (!Mv) return;

    // 평소 선호값으로 복귀(상황에 맞게 바꾸세요)
    Mv->bUseControllerDesiredRotation = false;
    Mv->bOrientRotationToMovement = true;
}

void UGA_MonsterAssemble::BeginGetUp(ACharacter* Chr)
{
    if (NS_AssemblePop || SFX_AssemblePop)
    {
        FVector FXLoc = GetFeetOrPelvisLoc(Chr);
        if (FHitResult H = TraceFloor(Chr, Chr->GetActorLocation(), FloorTraceDist, FloorTraceChannel);
            H.IsValidBlockingHit())
            FXLoc.Z = H.ImpactPoint.Z + 2.f;

        if (NS_AssemblePop)
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(Chr->GetWorld(), NS_AssemblePop, FXLoc);

        if (SFX_AssemblePop)
            UGameplayStatics::PlaySoundAtLocation(Chr, SFX_AssemblePop, FXLoc);

    }

    if (!Chr) return;
    USkeletalMeshComponent* Sk = GetMesh(Chr);
    UCapsuleComponent* Cap = GetCapsule(Chr);
    if (!Sk || !Cap) return;

    // 목표 Yaw (Pelvis 기준)
    float DesiredYaw = Chr->GetActorRotation().Yaw;
    if (int32 PelvisIdx = Sk->GetBoneIndex(PelvisBone); PelvisIdx != INDEX_NONE)
    {
        DesiredYaw = Sk->GetBoneQuaternion(PelvisBone).Rotator().Yaw;
    }

    // 목표 Loc(Z) = 바닥 위 캡슐 하프 + 오프셋
    FHitResult FloorHit = TraceFloor(Chr, Chr->GetActorLocation(), FloorTraceDist, FloorTraceChannel);
    const float Half = Cap->GetScaledCapsuleHalfHeight();
    FVector DesiredLoc = Chr->GetActorLocation();
    if (FloorHit.IsValidBlockingHit())
        DesiredLoc.Z = FloorHit.ImpactPoint.Z + Half + StandSnapUpOffset;

    // 시작/타깃 저장
    GetUpStartLoc = Chr->GetActorLocation();
    GetUpTargetLoc = DesiredLoc;
    GetUpStartYaw = Chr->GetActorRotation().Yaw;
    GetUpTargetYaw = DesiredYaw;

    // 조립이 끝났으니 물리는 OFF (혹시 남았으면)
    Sk->SetAllBodiesPhysicsBlendWeight(0.f);
    Sk->SetAllBodiesSimulatePhysics(false);

    GetUpElapsed = 0.f;
    GetUpDuration = FMath::Max(0.15f, GetUpTime);

    // 틱 시작
    Chr->GetWorldTimerManager().SetTimer(TH_GetUpTick, this, &UGA_MonsterAssemble::TickGetUp, TickInterval, true, 0.f);
}

void UGA_MonsterAssemble::TickGetUp()
{
    ACharacter* Chr = OwnerChar.Get();
    if (!Chr) return;

    const float DT = Chr->GetWorld()->GetDeltaSeconds();
    GetUpElapsed += DT;

    const float t = FMath::Clamp(GetUpElapsed / GetUpDuration, 0.f, 1.f);
    // 천천히 올라오게(느린 커브): EaseInOutCubic
    const float a = (t < 0.5f) ? 4.f * t * t * t : 1.f - FMath::Pow(-2.f * t + 2.f, 3.f) / 2.f;

    // 위치/회전 목표
    const FVector TargetLoc = FMath::Lerp(GetUpStartLoc, GetUpTargetLoc, a);
    const float   DeltaYaw = FMath::FindDeltaAngleDegrees(GetUpStartYaw, GetUpTargetYaw);
    const float   TargetYaw = FMath::UnwindDegrees(GetUpStartYaw + DeltaYaw * a);

    // 부드러운 추종 속도(느리게 하려면 여기 낮추면 됨)
    const float LocSpeed = 12.f; // ← 8~14 권장 (낮을수록 더 느리게 올라옴)
    const float RotSpeed = 10.f; // ← 8~12 권장

    const FVector  CurLoc = Chr->GetActorLocation();
    const FRotator CurRot = Chr->GetActorRotation();
    const FVector  NewLoc = FMath::VInterpTo(CurLoc, TargetLoc, DT, LocSpeed);
    const FRotator NewRot = FMath::RInterpTo(CurRot, FRotator(0.f, TargetYaw, 0.f), DT, RotSpeed);

    Chr->SetActorLocation(NewLoc, /*bSweep=*/false, nullptr, ETeleportType::None);
    Chr->SetActorRotation(NewRot, ETeleportType::None);

    if (t >= 1.f - KINDA_SMALL_NUMBER)
    {
        Chr->GetWorldTimerManager().ClearTimer(TH_GetUpTick);
        FinishGetUp(Chr);
    }
}

void UGA_MonsterAssemble::FinishGetUp(ACharacter* Chr)
{
    if (!Chr) return;

    USkeletalMeshComponent* Sk = GetMesh(Chr);
    UCapsuleComponent* Cap = GetCapsule(Chr);
    UCharacterMovementComponent* Mv = GetMove(Chr);
    if (!Sk || !Cap || !Mv) return;

    StopAssembleFX();
    // 미세 오차 보정(큰 오차만 텔레포트)
    const FVector DesiredLoc = GetUpTargetLoc;
    const float   DesiredYaw = GetUpTargetYaw;

    const float PosThresh = 1.0f;
    const float YawThresh = 1.0f;
    const FVector CurLoc = Chr->GetActorLocation();
    const FRotator CurRot = Chr->GetActorRotation();

    const bool bNeedPosSnap = (FVector::DistSquared(CurLoc, DesiredLoc) > PosThresh * PosThresh);
    const bool bNeedYawSnap = (FMath::Abs(FMath::FindDeltaAngleDegrees(CurRot.Yaw, DesiredYaw)) > YawThresh);

    if (bNeedPosSnap || bNeedYawSnap)
        Chr->TeleportTo(DesiredLoc, FRotator(0.f, DesiredYaw, 0.f), false, true);
    else
    {
        Chr->SetActorLocation(DesiredLoc, false, nullptr, ETeleportType::None);
        Chr->SetActorRotation(FRotator(0.f, DesiredYaw, 0.f), ETeleportType::None);
    }

    // (원래 StandUpFix에서 하던 마무리)
    Sk->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Cap->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    Sk->AttachToComponent(Cap, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    if (AMonsterCharacter* MC = Cast<AMonsterCharacter>(Chr))
    {
        Sk->SetRelativeLocation(MC->MeshInitRelLoc, false, nullptr, ETeleportType::TeleportPhysics);
        Sk->SetRelativeRotation(MC->MeshInitRelRot, false, nullptr, ETeleportType::TeleportPhysics);
        Sk->SetRelativeScale3D(MC->MeshInitRelScale);
    }

    Sk->SetAllPhysicsLinearVelocity(FVector::ZeroVector, false);
    Sk->SetAllPhysicsAngularVelocityInDegrees(FVector::ZeroVector, false);
    Sk->ResetAnimInstanceDynamics(ETeleportType::TeleportPhysics);
    Sk->ForceClothNextUpdateTeleportAndReset();
    Sk->RefreshBoneTransforms();

    RestoreRotateFlags(Chr);
    Cap->UpdateOverlaps();

    if (AAIController* AIC = GetAI(Chr))
        if (AIC->BrainComponent) AIC->BrainComponent->StartLogic();

    if (AMonsterCharacter* MC = Cast<AMonsterCharacter>(Chr))
    {
        MC->SetMonsterState(EMonsterState::CombatReady);
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

FVector UGA_MonsterAssemble::GetFeetOrPelvisLoc(ACharacter* Chr) const
{
    if (!Chr) return FVector::ZeroVector;

    USkeletalMeshComponent* Sk = GetMesh(Chr);
    if (!Sk) return Chr->GetActorLocation();

    static const FName FootL(TEXT("foot_l"));
    static const FName FootR(TEXT("foot_r"));

    if (Sk->DoesSocketExist(FootL) && Sk->DoesSocketExist(FootR))
    {
        const FVector L = Sk->GetSocketLocation(FootL);
        const FVector R = Sk->GetSocketLocation(FootR);
        return (L + R) * 0.5f;
    }

    if (Sk->GetBoneIndex(PelvisBone) != INDEX_NONE)
        return Sk->GetBoneLocation(PelvisBone);

    return Chr->GetActorLocation();
}

void UGA_MonsterAssemble::StartAssembleFX(ACharacter* Chr)
{
    if (!NS_AssembleLoop || AssembleFXComp || !Chr) return;

    USkeletalMeshComponent* Sk = GetMesh(Chr);
    if (!Sk) return;

    // 골반 본의 월드 위치/회전
    FVector  FXLoc = Sk->GetBoneLocation(PelvisBone);
    FXLoc.Z += AssembleFXZOffset;
    FRotator FXRot = Sk->GetBoneQuaternion(PelvisBone).Rotator();

    AssembleFXComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        Chr->GetWorld(),
        NS_AssembleLoop,
        FXLoc,
        FXRot,
        FVector(3.f),              // 스케일
        /*bAutoDestroy=*/false,
        /*bAutoActivate=*/true,
        ENCPoolMethod::None,
        /*bPreCullCheck=*/true
    );
}

void UGA_MonsterAssemble::StopAssembleFX()
{
    if (AssembleFXComp)
    {
        AssembleFXComp->Deactivate();
        AssembleFXComp->DestroyComponent();
        AssembleFXComp = nullptr;
    }
}