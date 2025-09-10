#pragma once
#include "Engine/DataAsset.h" 
#include "GameplayTagContainer.h"
#include "MonsterDefinition.generated.h"


USTRUCT(BlueprintType)
struct FAttackMontageEntry
{
    GENERATED_BODY()

    // �����Ϳ��� �ĺ�/���ÿ� Ű (��: "Light", "Heavy", "Rush", "AOE")
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName Key;

    // ���� ���� ��Ÿ��
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<class UAnimMontage> Montage;

    // (����) �� ��Ʈ������ �켱������ ����� ���ǵ�
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FName> Sections;

    // (����) ���� ���ξ�� �ڵ� �����ϰ� ���� �� (��: "Combo")
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName SectionPrefix;

    // (����) ���� ���� �� ����ġ
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Weight = 1;
};

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
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Attack")
    TArray<FAttackMontageEntry> AttackList;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    TSoftObjectPtr<class UAnimMontage> HitReactMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    TSoftObjectPtr<class UAnimMontage> DeathMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Tags")
    FGameplayTagContainer MonsterTags;

    // ������� ������ ����(100% ����)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drops")
    TSubclassOf<AActor> DropActorClass;
public:
    // Ű�� Ư�� ���� ã��
    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    bool FindAttackByKey(FName _key, class UAnimMontage*& _outMontage, FName& _outSection);

    // �ƹ� Ű�� �� �� ��� ���� ����
    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    bool PickRandomAttack(class UAnimMontage*& _outMontage, FName& _outSection);
};