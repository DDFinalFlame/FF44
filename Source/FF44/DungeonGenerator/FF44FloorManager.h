// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterSpawnInfo.h"
#include "FF44FloorManager.generated.h"

class AFF44DungeonGenerator;
class AFF44MonsterSpawner;
class AFF44InteractableSpawner;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFloorEvent, int32, FloorIndex);

UCLASS()
class FF44_API AFF44FloorManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AFF44FloorManager();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, Category = "Flow|Classes")
    TSubclassOf<AFF44DungeonGenerator> DungeonGeneratorClass;

    UPROPERTY(EditAnywhere, Category = "Flow|Classes")
    TSubclassOf<AFF44MonsterSpawner> MonsterSpawnerClass;

    UPROPERTY(EditAnywhere, Category = "Flow|Classes")
    TSubclassOf<AFF44InteractableSpawner> InteractableSpawnerClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow|Seed")
    int32 BaseSeed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow|Rules", meta = (ClampMin = "1"))
    int32 FloorsPerBossCycle = 5;

    UPROPERTY(BlueprintReadOnly, Category = "Flow|State")
    int32 CurrentFloor = 1;

    UPROPERTY(BlueprintReadOnly, Category = "Flow|State")
    bool bFloorReady = false;

    UPROPERTY(BlueprintReadOnly, Category = "Flow|Markers")
    TArray<FMonsterSpawnInfo> CachedMonsterMarkers;

    UPROPERTY(BlueprintReadOnly, Category = "Flow|Markers")
    TArray<FInteractableSpawnInfo> CachedInteractableMarkers;

public:
    // 외부 이벤트(UI에서 바인딩)
    UPROPERTY(BlueprintAssignable, Category = "Flow|Events")
    FFloorEvent OnFloorStarted;

    UPROPERTY(BlueprintAssignable, Category = "Flow|Events")
    FFloorEvent OnFloorReady;

    UPROPERTY(BlueprintAssignable, Category = "Flow|Events")
    FFloorEvent OnFloorEnded;

public:
    UFUNCTION(BlueprintCallable, Category = "Flow")
    void StartRun(int32 InBaseSeed, int32 StartFloor = 1);

    UFUNCTION(BlueprintCallable, Category = "Flow")
    void NextFloor();

public:
    UPROPERTY()
    AFF44DungeonGenerator* Dungeon = nullptr;

    UPROPERTY()
    AFF44MonsterSpawner* MonsterSpawner = nullptr;

    UPROPERTY()
    AFF44InteractableSpawner* InteractableSpawner = nullptr;

    UFUNCTION()
    void HandleDungeonComplete();

    UFUNCTION()
    void HandleMonsterSpawnComplete();

    UFUNCTION()
    void HandleInteractableSpawnComplete();

    void StartFloorInternal();
    void CleanupFloor();
    bool IsBossFloor() const { return (CurrentFloor % FloorsPerBossCycle) == 0; }
    int32 SeedForFloor() const { return ::HashCombine(::GetTypeHash(BaseSeed), ::GetTypeHash(CurrentFloor)); }

    // 추가: 두 스텝의 완료 여부만 추적
    bool bMonstersDone = false;
    bool bInteractablesDone = false;

    // 완료 체크용 헬퍼
    void TryFinishFloorReady();
};
