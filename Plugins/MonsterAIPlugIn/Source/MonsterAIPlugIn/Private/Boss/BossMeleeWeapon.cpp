// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss/BossMeleeWeapon.h"
#include "Components/BoxComponent.h"

ABossMeleeWeapon::ABossMeleeWeapon()
{
    Hitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("Hitbox"));
    RootComponent = Hitbox;
    Hitbox->InitBoxExtent(BoxExtent);


    RegisterHitbox(Hitbox);
}
