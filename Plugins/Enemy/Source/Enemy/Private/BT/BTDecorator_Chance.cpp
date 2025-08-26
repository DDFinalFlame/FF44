// Fill out your copyright notice in the Description page of Project Settings.


#include "BT/BTDecorator_Chance.h"


bool UBTDecorator_Chance::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	//bool result = ;

	//if (result)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, TEXT(" true"));
	//}
	//else
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, TEXT("false"));
	//}
	return ChangeRate > FMath::RandRange(1, 100);
}
