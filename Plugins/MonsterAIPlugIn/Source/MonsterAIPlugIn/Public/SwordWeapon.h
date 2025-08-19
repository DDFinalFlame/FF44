// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MonsterBaseWeapon.h"
#include "SwordWeapon.generated.h"

class USkeletalMeshComponent;
class UBoxComponent;
UCLASS()
class MONSTERAIPLUGIN_API ASwordWeapon : public AMonsterBaseWeapon
{
    GENERATED_BODY()
public:
    ASwordWeapon();

protected:
    UPROPERTY(VisibleAnywhere)
    USkeletalMeshComponent* SwordMesh = nullptr;

    UPROPERTY(VisibleAnywhere)
    UBoxComponent* SwordHitbox = nullptr;
};
