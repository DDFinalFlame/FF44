// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MonsterCharacter.h"
#include "BossCharacter.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERAIPLUGIN_API ABossCharacter : public AMonsterCharacter
{
	GENERATED_BODY()
public:
    ABossCharacter();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FName WeaponAttachSocketName = FName(TEXT("RockAttachPoint"));
};
