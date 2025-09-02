#pragma once

#include "CoreMinimal.h"
#include "MonsterCharacter.h"
#include "SkeletonHeadMonster.generated.h"

UCLASS()
class MONSTERAIPLUGIN_API ASkeletonHeadMonster : public AMonsterCharacter
{
    GENERATED_BODY()
public:
    ASkeletonHeadMonster();

protected:
    virtual void BeginPlay() override;

    virtual void Tick(float _dt) override;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FName WeaponAttachSocketName = FName(TEXT("HeadSocket"));	


    UPROPERTY(EditDefaultsOnly, Category = "Assemble")
    TSubclassOf<class UGameplayAbility> AssembleAbilityClass;

    UPROPERTY(EditDefaultsOnly, Category = "Assemble")
    float AssembleTriggerDistance = 1200.f;

    UPROPERTY()
    bool bAssembleRequested = false;
};
