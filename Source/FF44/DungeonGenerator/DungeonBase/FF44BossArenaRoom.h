// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DungeonGenerator/DungeonBase/FF44RoomBase.h"
#include "FF44BossArenaRoom.generated.h"

class UArrowComponent;

UCLASS()
class FF44_API AFF44BossArenaRoom : public AFF44RoomBase
{
    GENERATED_BODY()

public:
    AFF44BossArenaRoom();

public:
    UPROPERTY(VisibleAnywhere, Category = "Room|Folders")
    USceneComponent* PlayerStart;

    UPROPERTY(VisibleAnywhere, Category = "BossArena")
    UArrowComponent* PlayerStartPoint;

public:
    UFUNCTION(BlueprintCallable, Category = "BossArena")
    bool GetPlayerStartTransform(FTransform& OutTransform) const;
};