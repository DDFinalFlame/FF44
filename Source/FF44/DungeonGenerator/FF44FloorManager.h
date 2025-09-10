// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnInfo.h"
#include "FF44FloorManager.generated.h"

class AFF44DungeonGenerator;
class AFF44MonsterSpawner;
class AFF44InteractableSpawner;
class AFF44Portal;
class UAudioComponent;
class USoundBase;

class UFF44GameInstance;
class UAbilitySystemComponent;
class UFF44InventoryComponent;
class ABasePlayer;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFloorEvent, int32, FloorIndex);

UCLASS()
class FF44_API AFF44FloorManager : public AActor
{
    GENERATED_BODY()

public:
    AFF44FloorManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    /* ===========================
       Config (Class references)
       =========================== */
    UPROPERTY(EditAnywhere, Category = "Flow|Classes")
    TSubclassOf<AFF44DungeonGenerator> DungeonGeneratorClass;

    UPROPERTY(EditAnywhere, Category = "Flow|Classes")
    TSubclassOf<AFF44MonsterSpawner> MonsterSpawnerClass;

    UPROPERTY(EditAnywhere, Category = "Flow|Classes")
    TSubclassOf<AFF44InteractableSpawner> InteractableSpawnerClass;

    /* ===========================
       Run settings
       =========================== */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow|Seed")
    int32 BaseSeed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow|Rules", meta = (ClampMin = "1"))
    int32 FloorsPerBossCycle = 5;

    /* ===========================
       Runtime state
       =========================== */
    UPROPERTY(BlueprintReadOnly, Category = "Flow|State")
    int32 CurrentFloor = 0;

    UFUNCTION(BlueprintPure, Category = "FF44|Floor")
    int32 GetCurrentFloor() const { return CurrentFloor; }

    UPROPERTY(BlueprintReadOnly, Category = "Flow|State")
    bool bFloorReady = false;

    UPROPERTY(BlueprintReadOnly, Category = "Flow|Markers")
    TArray<FMonsterSpawnInfo> CachedMonsterMarkers;

    UPROPERTY(BlueprintReadOnly, Category = "Flow|Markers")
    TArray<FInteractableSpawnInfo> CachedInteractableMarkers;

    /* ===========================
       Blueprint events
       =========================== */
    UPROPERTY(BlueprintAssignable, Category = "Flow|Events")
    FFloorEvent OnFloorStarted;

    UPROPERTY(BlueprintAssignable, Category = "Flow|Events")
    FFloorEvent OnFloorReady;

    UPROPERTY(BlueprintAssignable, Category = "Flow|Events")
    FFloorEvent OnFloorEnded;

    /* ===========================
       Public API
       =========================== */
    UFUNCTION(BlueprintCallable, Category = "Flow")
    void StartRun(int32 InBaseSeed, int32 StartFloor = 1);

    UFUNCTION(BlueprintCallable, Category = "Flow")
    void NextFloor();

    UPROPERTY(EditAnywhere, Category = "Maps")
    TSoftObjectPtr<UWorld> LobbyLevel;

    /* ===========================
       Public API
       =========================== */

public:
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayCurrentFloorMusic();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopMusic(bool bImmediate = false);

protected:
    UPROPERTY(EditAnywhere, Category = "Audio")
    float MusicFadeInTime = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Audio")
    float MusicFadeOutTime = 0.3f;

    UPROPERTY(VisibleAnywhere, Category = "Audio")
    UAudioComponent* MusicComponent = nullptr;

    UPROPERTY(EditAnywhere, Category = "Audio")
    bool bMuteMusicInBossArena = true;

    UPROPERTY(Transient)
    bool bInBossArena = false;

private:
    FName ResolvePortalTag(class AFF44Portal* Portal, FName Passed) const;

    FDelegateHandle PortalSpawnedHandle;

    void BindSinglePortal(AFF44Portal* P);
    void OnActorSpawned(AActor* A);

    /* ===========================
       Internals
       =========================== */
    // Active actors (kept as UPROPERTY to prevent GC)
    UPROPERTY()
    AFF44DungeonGenerator* Dungeon = nullptr;

    UPROPERTY()
    AFF44MonsterSpawner* MonsterSpawner = nullptr;

    UPROPERTY()
    AFF44InteractableSpawner* InteractableSpawner = nullptr;

    // Phase flags for parallel completion
    bool bMonstersDone = false;
    bool bInteractablesDone = false;

    // Bound portals (to unbind on cleanup)
    UPROPERTY(Transient)
    TArray<TWeakObjectPtr<AFF44Portal>> BoundPortals;

    // Flow helpers
    void StartFloorInternal();
    void CleanupFloor();
    void TryFinishFloorReady();

    // Step handlers
    UFUNCTION()
    void HandleDungeonComplete();

    UFUNCTION()
    void HandleMonsterSpawnComplete();

    UFUNCTION()
    void HandleInteractableSpawnComplete();

    // Portal binding/handling
    void BindToPortals();
    void UnbindFromPortals();

    UFUNCTION()
    void HandlePortalInteracted(AFF44Portal* Portal, FName PortalTag);

    // Inline helpers
    bool IsBossFloor() const { return (CurrentFloor % FloorsPerBossCycle) == 0; }
    int32 SeedForFloor() const { return ::HashCombine(::GetTypeHash(BaseSeed), ::GetTypeHash(CurrentFloor)); }

    UFUNCTION(BlueprintCallable, Category = "Player")
    void CapturePlayerStateForInstance();
};