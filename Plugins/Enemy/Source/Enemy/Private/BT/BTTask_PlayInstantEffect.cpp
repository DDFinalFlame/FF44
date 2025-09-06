// Fill out your copyright notice in the Description page of Project Settings.


#include "BT/BTTask_PlayInstantEffect.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"

EBTNodeResult::Type UBTTask_PlayInstantEffect::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // Valid üũ
    APawn* ControlledPawn = OwnerComp.GetAIOwner()->GetPawn();
    if (!ControlledPawn) return EBTNodeResult::Failed;

    UAbilitySystemComponent* ASC = ControlledPawn->FindComponentByClass<UAbilitySystemComponent>();
    if (!ASC) return EBTNodeResult::Failed;

    // FX ����
    if (ASC && EffectClass)
    {
        FGameplayEffectContextHandle EffectContextHandle = ASC->MakeEffectContext();
        FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, 1.0f, EffectContextHandle);
        ASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), ASC);
    }

    return EBTNodeResult::Succeeded;

}
