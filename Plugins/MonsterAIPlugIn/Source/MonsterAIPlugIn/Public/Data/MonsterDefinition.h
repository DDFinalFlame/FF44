#pragma once
#include "Engine/DataAsset.h" 
#include "GameplayTagContainer.h"
#include "MonsterDefinition.generated.h"


USTRUCT(BlueprintType)
struct FAttackMontageEntry
{
    GENERATED_BODY()

    // 에디터에서 식별/선택용 키 (예: "Light", "Heavy", "Rush", "AOE")
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName Key;

    // 실제 공격 몽타주
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<class UAnimMontage> Montage;

    // (선택) 이 엔트리에서 우선적으로 사용할 섹션들
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FName> Sections;

    // (선택) 섹션 접두어로 자동 수집하고 싶을 때 (예: "Combo")
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName SectionPrefix;

    // (선택) 랜덤 선택 시 가중치
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Weight = 1;
};

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

    // 전투 몽타주
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Attack")
    TArray<FAttackMontageEntry> AttackList;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    TSoftObjectPtr<class UAnimMontage> HitReactMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    TSoftObjectPtr<class UAnimMontage> DeathMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Tags")
    FGameplayTagContainer MonsterTags;

    // 드랍으로 스폰할 액터(100% 스폰)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drops")
    TSubclassOf<AActor> DropActorClass;
public:
    // 키로 특정 공격 찾기
    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    bool FindAttackByKey(FName _key, class UAnimMontage*& _outMontage, FName& _outSection);

    // 아무 키도 안 준 경우 랜덤 선택
    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    bool PickRandomAttack(class UAnimMontage*& _outMontage, FName& _outSection);
};