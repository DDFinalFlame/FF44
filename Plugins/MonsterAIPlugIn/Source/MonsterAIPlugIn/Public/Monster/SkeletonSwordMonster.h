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
    virtual void Tick(float DeltaSeconds) override;

    // �״� ���� ����
    UPROPERTY(EditAnywhere, Category = "Audio")
    USoundBase* DeathSound = nullptr;

    bool bDeathSoundPlayed = false;
    // �� ���Ͱ� ���⸦ ���� ���ϸ�(Į)
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    FName WeaponAttachSocketName = FName(TEXT("SwordSocket"));
	
};
