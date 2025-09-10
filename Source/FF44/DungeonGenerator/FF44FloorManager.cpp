// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator/FF44FloorManager.h"
#include "DungeonGenerator/FF44DungeonGenerator.h"
#include "DungeonGenerator/FF44MonsterSpawner.h"
#include "DungeonGenerator/FF44InteractableSpawner.h"
#include "Interactable/FF44Portal.h"
#include "Kismet/GameplayStatics.h"
#include "DungeonGenerator/DungeonBase/FF44RoomBase.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

AFF44FloorManager::AFF44FloorManager()
{
    PrimaryActorTick.bCanEverTick = true;

    MusicComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MusicComponent"));
    MusicComponent->bAutoActivate = false;
    MusicComponent->bAllowSpatialization = false;
    MusicComponent->bIsUISound = true;
    MusicComponent->SetupAttachment(RootComponent);
}

void AFF44FloorManager::BeginPlay()
{
	Super::BeginPlay();

    if (UWorld* W = GetWorld())
    {
        PortalSpawnedHandle = W->AddOnActorSpawnedHandler(
            FOnActorSpawned::FDelegate::CreateUObject(this, &AFF44FloorManager::OnActorSpawned));
    }

	StartRun(BaseSeed, 1);
}

void AFF44FloorManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UWorld* W = GetWorld())
    {
        if (PortalSpawnedHandle.IsValid())
        {
            W->RemoveOnActorSpawnedHandler(PortalSpawnedHandle);
            PortalSpawnedHandle.Reset();
        }
    }
    Super::EndPlay(EndPlayReason);
}

void AFF44FloorManager::StartRun(int32 InBaseSeed, int32 StartFloor)
{
    BaseSeed = InBaseSeed;
    CurrentFloor = StartFloor;
    NextFloor();
}

void AFF44FloorManager::NextFloor()
{
    CleanupFloor();
    StartFloorInternal();
}

void AFF44FloorManager::PlayCurrentFloorMusic()
{
    if (!MusicComponent || !Dungeon) return;

    USoundBase* NewMusic = Dungeon->GetLoadedThemeMusic();
    if (!NewMusic) { StopMusic(true); return; }

    MusicComponent->SetSound(NewMusic);
    if (MusicFadeInTime > 0.f)
    {
        MusicComponent->FadeIn(MusicFadeInTime, 1.f);
    }
    else
    {
        MusicComponent->Play(0.f);
    }
}

void AFF44FloorManager::StopMusic(bool bImmediate)
{
    if (!MusicComponent) return;

    if (MusicComponent->IsPlaying())
    {
        if (!bImmediate && MusicFadeOutTime > 0.f)
        {
            MusicComponent->FadeOut(MusicFadeOutTime, 0.f);
        }
        else
        {
            MusicComponent->Stop();
        }
    }
}

FName AFF44FloorManager::ResolvePortalTag(AFF44Portal* Portal, FName Passed) const
{
    if (!Passed.IsNone())
        return Passed;

    static const FName N(TEXT("PortalNext"));
    static const FName B(TEXT("PortalBoss"));
    static const FName L(TEXT("PortalLobby"));

    // 1) Actor Tags
    if (Portal)
    {
        for (const FName& T : Portal->Tags)
        {
            if (T == N || T == B || T == L) return T;
        }

        // 2) 주요 컴포넌트의 ComponentTags도 확인(콜리전/트리거 등에 종종 태그를 다는 경우)
        TArray<UActorComponent*> Comps = Portal->GetComponents().Array();
        for (UActorComponent* C : Comps)
        {
            for (const FName& T : C->ComponentTags)
            {
                if (T == N || T == B || T == L) return T;
            }
        }
    }

    return NAME_None;
}

void AFF44FloorManager::BindSinglePortal(AFF44Portal* P)
{
    if (!P) return;

    if (!P->OnPortalInteracted.IsAlreadyBound(this, &AFF44FloorManager::HandlePortalInteracted))
    {
        P->OnPortalInteracted.AddDynamic(this, &AFF44FloorManager::HandlePortalInteracted);
    }
}

void AFF44FloorManager::OnActorSpawned(AActor* A)
{
    if (AFF44Portal* P = Cast<AFF44Portal>(A))
    {
        BindSinglePortal(P);
        BoundPortals.Add(P);
    }
}

void AFF44FloorManager::StartFloorInternal()
{
    OnFloorStarted.Broadcast(CurrentFloor);

    bInBossArena = false;
    StopMusic(false);

    if (!DungeonGeneratorClass) return;

    const FTransform SpawnTM = FTransform::Identity;
    Dungeon = GetWorld()->SpawnActorDeferred<AFF44DungeonGenerator>(DungeonGeneratorClass, SpawnTM, this, nullptr,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        //SpawnActor<AFF44DungeonGenerator>(DungeonGeneratorClass);
    if (!Dungeon) return;

    Dungeon->bIsBossFloor = IsBossFloor();
    Dungeon->ApplyThemeForFloor(CurrentFloor);
    // 필요한 경우: Dungeon->GenerationSeed = SeedForFloor();

    UGameplayStatics::FinishSpawningActor(Dungeon, SpawnTM);

    Dungeon->OnDungeonComplete.AddDynamic(this, &AFF44FloorManager::HandleDungeonComplete);
}

void AFF44FloorManager::CleanupFloor()
{
    UnbindFromPortals();

    if (MonsterSpawner)
    {
        MonsterSpawner->CleanupSpawned();
        MonsterSpawner->Destroy();
        MonsterSpawner = nullptr;
    }

    if (InteractableSpawner)
    {
        InteractableSpawner->CleanupSpawned();
        InteractableSpawner->Destroy();
        InteractableSpawner = nullptr;
    }

    if (Dungeon)
    {
        Dungeon->ClearDungeonContents();
        Dungeon->Destroy();
        Dungeon = nullptr;
    }

    bFloorReady = false;
    bMonstersDone = false;
    bInteractablesDone = false;

    CachedMonsterMarkers.Reset();
    CachedInteractableMarkers.Reset();
}

void AFF44FloorManager::TryFinishFloorReady()
{
    if (bMonstersDone && bInteractablesDone && !bFloorReady)
    {
        bFloorReady = true;
        OnFloorReady.Broadcast(CurrentFloor);
    }
}

void AFF44FloorManager::HandleDungeonComplete()
{
    if (!Dungeon) return;

    CachedMonsterMarkers = Dungeon->GetMonsterSpawnMarkers();
    CachedInteractableMarkers = Dungeon->GetInteractableSpawnMarkers();

    if (MonsterSpawnerClass && !MonsterSpawner)
    {
        MonsterSpawner = GetWorld()->SpawnActor<AFF44MonsterSpawner>(MonsterSpawnerClass);
        if (MonsterSpawner)
        {
            MonsterSpawner->OnSpawnComplete.AddDynamic(this, &AFF44FloorManager::HandleMonsterSpawnComplete);
        }
    }
    if (MonsterSpawner && CachedMonsterMarkers.Num() > 0)
    {
        MonsterSpawner->SpawnFromMarkers(CachedMonsterMarkers);
    }
    else
    {
        bMonstersDone = true;
    }

    if (InteractableSpawnerClass && !InteractableSpawner)
    {
        InteractableSpawner = GetWorld()->SpawnActor<AFF44InteractableSpawner>(InteractableSpawnerClass);
        if (InteractableSpawner)
        {
            InteractableSpawner->OnSpawnComplete.AddDynamic(this, &AFF44FloorManager::HandleInteractableSpawnComplete);
        }
    }
    if (InteractableSpawner && CachedInteractableMarkers.Num() > 0)
    {
        InteractableSpawner->SpawnFromMarkers(CachedInteractableMarkers, SeedForFloor() + 17);
    }
    else
    {
        bInteractablesDone = true;
    }

    TryFinishFloorReady();

    if (bInBossArena && bMuteMusicInBossArena)
    {
        StopMusic(true);
        return;
    }

    PlayCurrentFloorMusic();
}

void AFF44FloorManager::HandleMonsterSpawnComplete()
{
    bMonstersDone = true;

    TryFinishFloorReady();
}

void AFF44FloorManager::HandleInteractableSpawnComplete()
{
    bInteractablesDone = true;
    BindToPortals();

    TryFinishFloorReady();
}

void AFF44FloorManager::BindToPortals()
{
    UnbindFromPortals();

    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFF44Portal::StaticClass(), Found);

    for (AActor* A : Found)
    {
        if (AFF44Portal* P = Cast<AFF44Portal>(A))
        {
            P->OnPortalInteracted.AddDynamic(this, &AFF44FloorManager::HandlePortalInteracted);
            BoundPortals.Add(P);
        }
    }
}

void AFF44FloorManager::UnbindFromPortals()
{
    for (TWeakObjectPtr<AFF44Portal>& W : BoundPortals)
    {
        if (AFF44Portal* P = W.Get())
        {
            P->OnPortalInteracted.RemoveDynamic(this, &AFF44FloorManager::HandlePortalInteracted);
        }
    }

    BoundPortals.Empty();
}

void AFF44FloorManager::HandlePortalInteracted(AFF44Portal* Portal, FName PortalTag)
{
    const FName Tag = ResolvePortalTag(Portal, PortalTag);

    if (Tag == TEXT("PortalLobby"))
    {
        if (MonsterSpawner)      MonsterSpawner->CleanupSpawned();
        if (InteractableSpawner) InteractableSpawner->CleanupSpawned();

        UGameplayStatics::OpenLevelBySoftObjectPtr(this, LobbyLevel);
        return;
    }

    if (Tag == TEXT("PortalBoss"))
    {
        if (!Dungeon || !IsBossFloor()) { return; }

        if (MonsterSpawner)      MonsterSpawner->CleanupSpawned();
        if (InteractableSpawner) InteractableSpawner->CleanupSpawned();

        if (bMuteMusicInBossArena)
        {
            bInBossArena = true;
        }

        static const FName FnName = TEXT("EnterBossArena");
        if (Dungeon->GetClass()->FindFunctionByName(FnName))
        {
            Dungeon->CallFunctionByNameWithArguments(*FnName.ToString(), *GLog, nullptr, true);
        }
        return;
    }

    if (Tag == TEXT("PortalNext"))
    {
        OnFloorEnded.Broadcast(CurrentFloor);
        CurrentFloor = FMath::Max(1, CurrentFloor + 1);
        NextFloor();
        return;
    }
}