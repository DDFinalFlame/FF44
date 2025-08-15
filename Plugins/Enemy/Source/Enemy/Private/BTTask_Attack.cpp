#include "BTTask_Attack.h"

#include "AIController.h"
#include "BaseEnemy.h"

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return EBTNodeResult::Failed;

    ABaseEnemy* Enemy = Cast<ABaseEnemy>(AICon->GetPawn());
    if (!Enemy) return EBTNodeResult::Failed;

    CachedASC = Enemy->FindComponentByClass<UAbilitySystemComponent>();
    if (!CachedASC) return EBTNodeResult::Failed;

    // Ability ���� ��������Ʈ ���
    AbilityEndedDelegateHandle = CachedASC->OnAbilityEnded.AddUObject(this, &UBTTask_Attack::OnAbilityEnded);
    CachedOwnerComp = &OwnerComp;

    // Enemy Ŭ���� ���� Ability �ߵ� ��û
    if (!Enemy->RequestAttack())
    {
        CachedASC->OnAbilityEnded.Remove(AbilityEndedDelegateHandle);
        return EBTNodeResult::Failed;
    }

    return EBTNodeResult::InProgress;
}

void UBTTask_Attack::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
    if (CachedASC)
    {
        CachedASC->OnAbilityEnded.Remove(AbilityEndedDelegateHandle);
    }

    CachedASC = nullptr;
    CachedOwnerComp = nullptr;

    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBTTask_Attack::OnAbilityEnded(const FAbilityEndedData& EndData)
{
    if (!CachedOwnerComp) return;

    // Enemy�� �ߵ��ϴ� Ability�� üũ
    AAIController* AICon = CachedOwnerComp->GetAIOwner();
    if (!AICon) return;
    ABaseEnemy* Enemy = Cast<ABaseEnemy>(AICon->GetPawn());
    if (!Enemy) return;

    if (EndData.AbilityThatEnded && EndData.AbilityThatEnded->GetClass() == Enemy->PerformAttackAbility)
    {
        CachedASC->OnAbilityEnded.Remove(AbilityEndedDelegateHandle);
        FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
    }
}
