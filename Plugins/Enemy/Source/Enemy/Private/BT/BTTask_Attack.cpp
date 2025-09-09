#include "BT/BTTask_Attack.h"

#include "AIController.h"
#include "BaseEnemy.h"

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Start Attack Task"));

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return EBTNodeResult::Failed;

	ABaseEnemy* Enemy = Cast<ABaseEnemy>(AICon->GetPawn());
	if (!Enemy) return EBTNodeResult::Failed;

	CachedASC = Enemy->FindComponentByClass<UAbilitySystemComponent>();
	if (!CachedASC) return EBTNodeResult::Failed;

	// Ability 종료 델리게이트 등록
	AbilityEndedDelegateHandle = CachedASC->OnAbilityEnded.AddUObject(this, &UBTTask_Attack::OnAbilityEnded);
	CachedOwnerComp = &OwnerComp;

	// Enemy 클래스 통해 Ability 발동 요청
	if (!Enemy->RequestAbilityByTag(AbilityTag))
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

	// Enemy가 발동하는 Ability만 체크
	AAIController* AICon = CachedOwnerComp->GetAIOwner();
	if (!AICon) return;
	ABaseEnemy* Enemy = Cast<ABaseEnemy>(AICon->GetPawn());
	if (!Enemy) return;

	if (EndData.AbilityThatEnded)
	{
		if (EndData.AbilityThatEnded->GetAssetTags().HasTag(AbilityTag))
		{
			CachedASC->OnAbilityEnded.Remove(AbilityEndedDelegateHandle);
			FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
		}
	}
}
