#include "BT/BTDecorator_IsTargetUnblocked.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

bool UBTDecorator_IsTargetUnblocked::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return false;

    APawn* ControlledPawn = AICon->GetPawn();
    if (!ControlledPawn) return false;

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp) return false;

    AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));
    if (!TargetActor) return false;

    return IsTargetUnblocked(ControlledPawn, TargetActor);
}

bool UBTDecorator_IsTargetUnblocked::IsTargetUnblocked(APawn* Observer, AActor* Target) const
{
    if (!Observer || !Target) return false;

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Observer);
    Params.AddIgnoredActor(Target);

    bool bHit = Observer->GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Observer->GetActorLocation(),
        Target->GetActorLocation(),
        ECC_Visibility,
        Params
    );

    DrawDebugLine(Observer->GetWorld(), Observer->GetActorLocation(), Target->GetActorLocation(), bHit ? FColor::Red : FColor::Green, false, 1.f, 0, 2.f);

    return !bHit;
}
