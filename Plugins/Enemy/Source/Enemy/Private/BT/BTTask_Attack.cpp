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

	//// ���� ���
	//// Enemy Ŭ���� ���� Ability �ߵ� ��û
	//if (!Enemy->RequestAbilityByTag(AbilityTag))
	//{
	//	return EBTNodeResult::Failed;
	//}

	CachedSpecHandle = Enemy->RequestAbilityByTag(AbilityTag);
	if (CachedSpecHandle.IsValid())
	{
		// Ability ���� ��������Ʈ ���
		AbilityEndedDelegateHandle = CachedASC->OnAbilityEnded.AddUObject(this, &UBTTask_Attack::OnAbilityEnded);
		CachedOwnerComp = &OwnerComp;

		return EBTNodeResult::Succeeded;

	}
	else
	{
		return EBTNodeResult::Failed;
	}

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
	CachedSpecHandle = FGameplayAbilitySpecHandle();

	FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);

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

	if (EndData.AbilitySpecHandle == CachedSpecHandle)
	{
		CachedASC->OnAbilityEnded.Remove(AbilityEndedDelegateHandle);
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
	}

	if (EndData.AbilityThatEnded)
	{
		// ���� ���
		//if (EndData.AbilityThatEnded->GetAssetTags().HasTag(AbilityTag))
		//{
		//	CachedASC->OnAbilityEnded.Remove(AbilityEndedDelegateHandle);
		//	FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
		//}

		

	}
}
