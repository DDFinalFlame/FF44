// Fill out your copyright notice in the Description page of Project Settings.


#include "EQS_Context_AttackTarget.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void UEQS_Context_AttackTarget::ProvideContext(FEnvQueryInstance& QueryInstance,
                                               FEnvQueryContextData& ContextData) const
{
    UObject* QuerierObject = QueryInstance.Owner.Get();
    if (!QuerierObject) return;

    // AIController�� ���
    if (AAIController* AICon = Cast<AAIController>(QuerierObject))
    {
        if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
        {
            if (AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(TargetKeyName)))
            {
                UEnvQueryItemType_Actor::SetContextHelper(ContextData, TargetActor);
            }
        }
    }
    // Querier�� Pawn�� ���
    else if (APawn* Pawn = Cast<APawn>(QuerierObject))
    {
        if (AAIController* AICon2 = Cast<AAIController>(Pawn->GetController()))
        {
            if (UBlackboardComponent* BB = AICon2->GetBlackboardComponent())
            {
                if (AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(TargetKeyName)))
                {
                    UEnvQueryItemType_Actor::SetContextHelper(ContextData, TargetActor);
                }
            }
        }
    }
}
