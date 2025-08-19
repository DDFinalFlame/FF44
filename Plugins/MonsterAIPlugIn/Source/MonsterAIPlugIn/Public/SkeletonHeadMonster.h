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

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FName WeaponAttachSocketName = FName(TEXT("HeadSocket"));	
};
