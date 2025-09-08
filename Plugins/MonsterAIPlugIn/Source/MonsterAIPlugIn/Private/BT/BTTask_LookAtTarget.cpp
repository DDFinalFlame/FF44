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
    TimeoutSeconds = 0.f; // 0 = ��� �� ��
    bUseAIFocus = true;
    StartTime = 0.f;

    // Actor/Object ���
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
        AICon->SetFocus(Target); // tick���� ��� ���ŵ� ��ǥ
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

    // Ÿ�Ӿƿ�
    if (TimeoutSeconds > 0.f)
    {
        const float Now = Pawn->GetWorld()->GetTimeSeconds();
        if (Now - StartTime >= TimeoutSeconds)
        {
            FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
            return;
        }
    }

    // ����-��ǥ(Yaw) ������
    const float DeltaYaw = GetDeltaYawToTarget(Pawn, Target);
    const float Step = YawSpeedDeg * DeltaSeconds;

    // ���� �Ǵ�
    const float AbsDelta = FMath::Abs(DeltaYaw);
    if (bFinishWhenAligned && AbsDelta <= AcceptAngleDeg)
    {
        // ���������� ��Ȯ�� �����
        const FRotator Desired = UKismetMathLibrary::FindLookAtRotation(Pawn->GetActorLocation(), Target->GetActorLocation());
        FRotator NewRot = Pawn->GetActorRotation();
        NewRot.Yaw = Desired.Yaw;
        Pawn->SetActorRotation(NewRot);
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    // ȸ�� ����(ƽ���� ��ǥ ����)
    float YawMove = FMath::Clamp(DeltaYaw, -Step, Step);
    FRotator NewRot = Pawn->GetActorRotation();
    NewRot.Yaw = FMath::UnwindDegrees(NewRot.Yaw + YawMove);
    Pawn->SetActorRotation(NewRot);

    // Focus ��� �� ������ ��Ʈ�ѷ� DesiredRotation�� ����������,
    // ���� ȸ����Ű�� ������ �� �ﰢ���Դϴ�.
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
