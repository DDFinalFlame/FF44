// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MonsterCharacter.h"
#include "SkeletonSwordMonster.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERAIPLUGIN_API ASkeletonSwordMonster : public AMonsterCharacter
{
    GENERATED_BODY()
public:
    ASkeletonSwordMonster();

protected:
    virtual void BeginPlay() override;

    // 이 몬스터가 무기를 붙일 소켓명(칼)
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FName WeaponAttachSocketName = FName(TEXT("SwordSocket"));
	
};
