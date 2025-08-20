// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator/FF44DungeonGenerator.h"
#include "DungeonGenerator/DungeonBase/FF44StarterRoom.h"
#include "DungeonGenerator/DungeonBase/FF44RoomBase.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"

AFF44DungeonGenerator::AFF44DungeonGenerator()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFF44DungeonGenerator::BeginPlay()
{
    Super::BeginPlay();

    AFF44StarterRoom* Starter = nullptr;
    SpawnStarterRoom(Starter);
    SpawnPlayerAtStart(Starter);
    SpawnNextRoom();
}

void AFF44DungeonGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFF44DungeonGenerator::SpawnStarterRoom(AFF44StarterRoom*& OutStarter)
{
    OutStarter = nullptr;

    if (!StarterRoomClass)
    {
        return;
    }

    OutStarter = GetWorld()->SpawnActor<AFF44StarterRoom>(StarterRoomClass);
    OutStarter->ExitPoints->GetChildrenComponents(false, Exits);

    //Exits.Empty();
    //if (OutStarter->ExitPoints)
    //{
    //    OutStarter->ExitPoints->GetChildrenComponents(false, Exits);
    //}

    //SmallExits.Empty();
    //if (OutStarter->SmallExitPoints)
    //{
    //    OutStarter->SmallExitPoints->GetChildrenComponents(false, SmallExits);
    //}
}

void AFF44DungeonGenerator::SpawnPlayerAtStart(const AFF44StarterRoom* Starter)
{
    if (!Starter)
    {
        return;
    }

    FTransform StartT;
    Starter->GetPlayerStartTransform(StartT);

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (APawn* Pawn = PC->GetPawn())
    {
        Pawn->SetActorLocationAndRotation(StartT.GetLocation(), StartT.GetRotation());
    }
}

void AFF44DungeonGenerator::SpawnNextRoom()
{
    if (RoomsToSpawn <= 0) return;
    if (Exits.Num() == 0) return;

    SelectedExitPoint = Exits[rand() % Exits.Num()];
    const bool bSmallExit = SelectedExitPoint->ComponentHasTag(TEXT("Small"));

    const TArray<TSubclassOf<AFF44RoomBase>>& Pool = bSmallExit ? SmallRoomsToBeSpawned : RoomsToBeSpawned;
    if (Pool.Num() == 0)
    {
        GetWorld()->GetTimerManager().SetTimer(SpawnNextHandle, this, &AFF44DungeonGenerator::SpawnNextRoom, 0.01f, false);
        return;
    }

    LatestSpawnedRoom = GetWorld()->SpawnActor<AFF44RoomBase>(Pool[rand() % Pool.Num()]);
    if (!LatestSpawnedRoom)
    {
        GetWorld()->GetTimerManager().SetTimer(SpawnNextHandle, this, &AFF44DungeonGenerator::SpawnNextRoom, 0.01f, false);
        return;
    }

    LatestSpawnedRoom->SetActorLocation(SelectedExitPoint->GetComponentLocation());
    FRotator R = SelectedExitPoint->GetComponentRotation();
    R.Yaw += 90.f;
    LatestSpawnedRoom->SetActorRotation(R);

    if (RemoveOverlappingRooms())
    {
        LatestSpawnedRoom->Destroy();
        GetWorld()->GetTimerManager().SetTimer(SpawnNextHandle, this, &AFF44DungeonGenerator::SpawnNextRoom, 0.01f, false);
        return;
    }

    TArray<USceneComponent*> NewExits;
    if (LatestSpawnedRoom->ExitPoints)
        LatestSpawnedRoom->ExitPoints->GetChildrenComponents(false, NewExits);

    if (NewExits.Num() == 0 && RoomsToSpawn > 1)
    {
        LatestSpawnedRoom->Destroy();
        GetWorld()->GetTimerManager().SetTimer(SpawnNextHandle, this, &AFF44DungeonGenerator::SpawnNextRoom, 0.01f, false);
        return;
    }

    Exits.Remove(SelectedExitPoint);
    Exits.Append(NewExits);

    RoomsToSpawn--;
    if (RoomsToSpawn > 0)
    {
        GetWorld()->GetTimerManager().SetTimer(SpawnNextHandle, this, &AFF44DungeonGenerator::SpawnNextRoom, 0.01f, false);
    }
    else
    {
        SealRemainingExits();
    }
}

bool AFF44DungeonGenerator::RemoveOverlappingRooms()
{
    if (!LatestSpawnedRoom || !LatestSpawnedRoom->OverlapFolder) return false;

    TArray<USceneComponent*> OverlapBoxes;
    LatestSpawnedRoom->OverlapFolder->GetChildrenComponents(false, OverlapBoxes);

    for (USceneComponent* Comp : OverlapBoxes)
    {
        UBoxComponent* Box = Cast<UBoxComponent>(Comp);
        if (!Box) continue;

        Box->UpdateOverlaps();

        TArray<UPrimitiveComponent*> Hits;
        Box->GetOverlappingComponents(Hits);

        for (UPrimitiveComponent* Hit : Hits)
        {
            if (!Hit) continue;
            if (Hit->GetOwner() == LatestSpawnedRoom) continue;

            return true;
        }
    }
    return false;
}

void AFF44DungeonGenerator::SealRemainingExits()
{

    for (USceneComponent* ExitComp : Exits)
    {
        if (!ExitComp) continue;

        const FVector Loc = ExitComp->GetComponentLocation();

        FRotator Rot = ExitComp->GetComponentRotation();
        Rot.Yaw += 90.f;

        GetWorld()->SpawnActor<AActor>(ExitCapClass, Loc, Rot);
    }

    Exits.Empty();
}

TSubclassOf<AFF44RoomBase> AFF44DungeonGenerator::PickWeightedRoom(const TArray<TSubclassOf<AFF44RoomBase>>& Pool) const
{
    if (Pool.Num() == 0) return nullptr;

    int32 TotalWeight = 0;
    for (const TSubclassOf<AFF44RoomBase>& Cls : Pool)
    {
        if (!*Cls) continue;
        const AFF44RoomBase* CDO = Cls->GetDefaultObject<AFF44RoomBase>();
        const int32 W = CDO ? FMath::Max(0, CDO->SpawnWeight) : 0; // À½¼ö ¹æÁö
        TotalWeight += W;
    }

    if (TotalWeight <= 0)
    {
        return Pool[FMath::RandRange(0, Pool.Num() - 1)];
    }

    // ·ê·¿ ¼±ÅÃ
    int32 Pick = FMath::RandRange(1, TotalWeight);
    int32 Acc = 0;

    for (const TSubclassOf<AFF44RoomBase>& Cls : Pool)
    {
        if (!*Cls) continue;
        const AFF44RoomBase* CDO = Cls->GetDefaultObject<AFF44RoomBase>();
        const int32 W = CDO ? FMath::Max(0, CDO->SpawnWeight) : 0;
        Acc += W;
        if (Pick <= Acc)
        {
            return Cls;
        }
    }

    return Pool.Last();
}


