#include "BT/BTTask_LookAtTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"

UBTTask_LookAtTarget::UBTTask_LookAtTarget()
{
    NodeName = TEXT("Look At Target (Continuous)");
    bNotifyTick = true;
    bCreateNodeInstance = false;

    YawSpeedDeg = 240.f;
    AcceptAngleDeg = 5.f;
    bFinishWhenAligned = false;
    TimeoutSeconds = 0.f; // 0 = 사용 안 함
    bUseAIFocus = true;
    StartTime = 0.f;

    // Actor/Object 허용
    TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_LookAtTarget, TargetActorKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_LookAtTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return EBTNodeResult::Failed;

    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
    if (!Pawn) return EBTNodeResult::Failed;

    AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
    if (!Target) return EBTNodeResult::Failed;

    if (bUseAIFocus && AICon)
    {
        AICon->SetFocus(Target); // tick에서 계속 갱신될 목표
    }

    StartTime = Pawn->GetWorld()->GetTimeSeconds();
    return EBTNodeResult::InProgress;
}

static float GetDeltaYawToTarget(const AActor* Self, const AActor* Target)
{
    const FVector SelfLoc = Self->GetActorLocation();
    const FVector TargetLoc = Target->GetActorLocation();
    const FRotator Cur = Self->GetActorRotation();

    const FRotator Desired = UKismetMathLibrary::FindLookAtRotation(SelfLoc, TargetLoc);
    const float Delta = FMath::FindDeltaAngleDegrees(Cur.Yaw, Desired.Yaw);
    return Delta; // [-180,180]
}

void UBTTask_LookAtTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
    if (!BB || !Pawn)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
    if (!Target)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // 타임아웃
    if (TimeoutSeconds > 0.f)
    {
        const float Now = Pawn->GetWorld()->GetTimeSeconds();
        if (Now - StartTime >= TimeoutSeconds)
        {
            FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
            return;
        }
    }

    // 현재-목표(Yaw) 각도차
    const float DeltaYaw = GetDeltaYawToTarget(Pawn, Target);
    const float Step = YawSpeedDeg * DeltaSeconds;

    // 정렬 판단
    const float AbsDelta = FMath::Abs(DeltaYaw);
    if (bFinishWhenAligned && AbsDelta <= AcceptAngleDeg)
    {
        // 마지막으로 정확히 맞춰둠
        const FRotator Desired = UKismetMathLibrary::FindLookAtRotation(Pawn->GetActorLocation(), Target->GetActorLocation());
        FRotator NewRot = Pawn->GetActorRotation();
        NewRot.Yaw = Desired.Yaw;
        Pawn->SetActorRotation(NewRot);
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    // 회전 적용(틱마다 목표 갱신)
    float YawMove = FMath::Clamp(DeltaYaw, -Step, Step);
    FRotator NewRot = Pawn->GetActorRotation();
    NewRot.Yaw = FMath::UnwindDegrees(NewRot.Yaw + YawMove);
    Pawn->SetActorRotation(NewRot);

    // Focus 사용 시 엔진이 컨트롤러 DesiredRotation도 관리하지만,
    // 직접 회전시키는 구조가 더 즉각적입니다.
    if (bUseAIFocus && AICon)
    {
        AICon->SetFocus(Target);
    }
}

void UBTTask_LookAtTarget::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    if (bUseAIFocus)
    {
        if (AAIController* AICon = OwnerComp.GetAIOwner())
        {
            AICon->ClearFocus(EAIFocusPriority::Gameplay);
        }
    }
}
