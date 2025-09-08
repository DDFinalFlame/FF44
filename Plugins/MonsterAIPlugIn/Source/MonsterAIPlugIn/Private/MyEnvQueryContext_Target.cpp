// Fill out your copyright notice in the Description page of Project Settings.


#include "MyEnvQueryContext_Target.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void UMyEnvQueryContext_Target::ProvideContext(FEnvQueryInstance& QueryInstance,
    FEnvQueryContextData& ContextData) const
{
    UObject* QuerierObject = QueryInstance.Owner.Get();
    if (!QuerierObject) return;

    AActor* TargetActor = nullptr;

    if (AAIController* AICon = Cast<AAIController>(QuerierObject))
    {
        if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
        {
            TargetActor = Cast<AActor>(BB->GetValueAsObject(TargetKeyName));
        }
    }
    else if (APawn* Pawn = Cast<APawn>(QuerierObject))
    {
        if (AAIController* AICon2 = Cast<AAIController>(Pawn->GetController()))
        {
            if (UBlackboardComponent* BB = AICon2->GetBlackboardComponent())
            {
                TargetActor = Cast<AActor>(BB->GetValueAsObject(TargetKeyName));
            }
        }
    }

    if (TargetActor)
    {
        UEnvQueryItemType_Actor::SetContextHelper(ContextData, TargetActor);
    }
    else if (AActor* QuerierAsActor = Cast<AActor>(QuerierObject))
    {
        // 폴백: 타겟이 없으면 자기 자신
        UEnvQueryItemType_Actor::SetContextHelper(ContextData, QuerierAsActor);

        // 위치만 필요한 컨텍스트로 쓰고 싶다면 아래처럼:
        // UEnvQueryItemType_VectorBase::SetContextHelper(ContextData, QuerierAsActor->GetActorLocation());
    }
}