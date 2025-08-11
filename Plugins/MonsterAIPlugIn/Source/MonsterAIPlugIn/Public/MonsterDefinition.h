#pragma once
#include "Engine/DataAsset.h" 
#include "GameplayTagContainer.h"
#include "MonsterDefinition.generated.h"

UCLASS(BlueprintType)
class MONSTERAIPLUGIN_API UMonsterDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    // 숫자는 DataTable에서 찾을 RowName
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Data")
    FName StatRowName;

    // 에셋(Soft 권장)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Visual")
    TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Visual")
    TSoftClassPtr<UAnimInstance> AnimBlueprint;

    // AI
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|AI")
    TSoftObjectPtr<class UBehaviorTree> BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|AI")
    TSoftObjectPtr<class UBlackboardData> BlackboardAsset;

    // 부여할 Ability/초기화 GE
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|GAS")
    TArray<TSubclassOf<class UGameplayAbility>> AbilitiesToGrant;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|GAS")
    TSubclassOf<class UGameplayEffect> InitStatGE_SetByCaller;

    // (선택) 공격 몽타주 맵
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Combat")
    TMap<FName, TSoftObjectPtr<class UAnimMontage>> AttackMontages;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Tags")
    FGameplayTagContainer MonsterTags;
};