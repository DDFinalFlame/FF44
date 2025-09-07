// Fill out your copyright notice in the Description page of Project Settings.


#include "BT/BTTask_PlayEffect.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"

EBTNodeResult::Type UBTTask_PlayEffect::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // Valid 체크
    APawn* ControlledPawn = OwnerComp.GetAIOwner()->GetPawn();
    if (!ControlledPawn) return EBTNodeResult::Failed;

    UAbilitySystemComponent* ASC = ControlledPawn->FindComponentByClass<UAbilitySystemComponent>();
    if (!ASC) return EBTNodeResult::Failed;

    // FX 실행
    if (ASC && EffectClass)
    {
        FGameplayEffectContextHandle EffectContextHandle = ASC->MakeEffectContext();
        FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass,1.0f, EffectContextHandle);
        ASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), ASC);
    }

    return EBTNodeResult::Succeeded;
}
