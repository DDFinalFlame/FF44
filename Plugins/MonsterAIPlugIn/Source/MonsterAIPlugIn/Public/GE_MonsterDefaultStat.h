#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_MonsterDefaultStat.generated.h"

/*
 초기 스텟 부여
 */
UCLASS()
class MONSTERAIPLUGIN_API UGE_MonsterDefaultStat : public UGameplayEffect
{
	GENERATED_BODY()
	
public:
	UGE_MonsterDefaultStat();
};
