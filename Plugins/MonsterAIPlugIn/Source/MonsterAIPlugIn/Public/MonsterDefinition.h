#pragma once
#include "Engine/DataAsset.h" 
#include "GameplayTagContainer.h"
#include "MonsterDefinition.generated.h"

UCLASS(BlueprintType)
class MONSTERAIPLUGIN_API UMonsterDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    // ���ڴ� DataTable���� ã�� RowName
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Data")
    FName StatRowName;

    // ����(Soft ����)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Visual")
    TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Visual")
    TSoftClassPtr<UAnimInstance> AnimBlueprint;

    // AI
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|AI")
    TSoftObjectPtr<class UBehaviorTree> BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|AI")
    TSoftObjectPtr<class UBlackboardData> BlackboardAsset;

    // �ο��� Ability/�ʱ�ȭ GE
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|GAS")
    TArray<TSubclassOf<class UGameplayAbility>> AbilitiesToGrant;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|GAS")
    TSubclassOf<class UGameplayEffect> InitStatGE_SetByCaller;

    // ���� ��Ÿ��
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    TSoftObjectPtr<class UAnimMontage> AttackMontage; // "Light","Heavy" �� Ű�� ����

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    TSoftObjectPtr<class UAnimMontage> HitReactMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    TSoftObjectPtr<class UAnimMontage> DeathMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Tags")
    FGameplayTagContainer MonsterTags;
};