// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MonsterBaseWeapon.h"
#include "HeadbuttWeapon.generated.h"

class UBoxComponent;
UCLASS()
class MONSTERAIPLUGIN_API AHeadbuttWeapon : public AMonsterBaseWeapon
{
	GENERATED_BODY()
public:
    AHeadbuttWeapon();



protected:
    UPROPERTY(VisibleAnywhere)
    UBoxComponent* HeadHitbox = nullptr;
};
