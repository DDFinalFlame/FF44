// Fill out your copyright notice in the Description page of Project Settings.


#include "BT/EQS_Context_LastHeard.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"

static const FName KEY_LastHeardLocation(TEXT("LastHeardLocation"));

void UEQS_Context_LastHeard::ProvideContext(FEnvQueryInstance& QueryInstance,
    FEnvQueryContextData& ContextData) const
{
    AAIController* AICon = Cast<AAIController>(QueryInstance.Owner.Get());
    if (!AICon) return;

    UBlackboardComponent* BB = AICon->GetBlackboardComponent();
    if (!BB) return;

    const FVector Loc = BB->GetValueAsVector(KEY_LastHeardLocation);

    ContextData.ValueType = UEnvQueryItemType_VectorBase::StaticClass();
    ContextData.RawData.Reset();
    ContextData.RawData.AddUninitialized(sizeof(FVector));

    // RawData를 FVector 포인터로 캐스팅해서 직접 기록
    *reinterpret_cast<FVector*>(ContextData.RawData.GetData()) = Loc;
}