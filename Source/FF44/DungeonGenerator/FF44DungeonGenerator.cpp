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

    SpawnStarterRoom(StarterRoomRef);
    SpawnPlayerAtStart(StarterRoomRef);
    SpawnNextRoom();
}

void AFF44DungeonGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFF44DungeonGenerator::SpawnStarterRoom(AFF44StarterRoom*& OutStarter)
{
    OutStarter = nullptr;
    if (!StarterRoomClass) return;

    OutStarter = GetWorld()->SpawnActor<AFF44StarterRoom>(StarterRoomClass);
    StarterRoomRef = OutStarter;

    if (OutStarter && OutStarter->ExitPoints)
    {
        OutStarter->ExitPoints->GetChildrenComponents(false, Exits);
    }
}

void AFF44DungeonGenerator::SpawnPlayerAtStart(const AFF44StarterRoom* Starter)
{
    if (!Starter) return;

    FTransform StartT;
    Starter->GetPlayerStartTransform(StartT);

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (APawn* Pawn = PC->GetPawn())
        {
            Pawn->SetActorLocationAndRotation(StartT.GetLocation(), StartT.GetRotation());

            PC->SetControlRotation(StartT.Rotator());
        }
    }
}

void AFF44DungeonGenerator::SpawnNextRoom()
{
    if (RoomsToSpawn <= 0 && Exits.Num() == 0) return;
    if (Exits.Num() == 0) return;

    SelectedExitPoint = Exits[FMath::RandRange(0, Exits.Num() - 1)];
    const bool bSmallExit = SelectedExitPoint->ComponentHasTag(TEXT("Small"));

    const TArray<TSubclassOf<AFF44RoomBase>>& Pool = bSmallExit ? SmallRoomsToBeSpawned : RoomsToBeSpawned;
    if (Pool.Num() == 0)
    {
        GetWorld()->GetTimerManager().SetTimer(SpawnNextHandle, this, &AFF44DungeonGenerator::SpawnNextRoom, RoomSpawnInterval, false);
        return;
    }

    // 가중치 쓰려면 아래 두 줄만 교체
    // TSubclassOf<AFF44RoomBase> Chosen = PickWeightedRoom(Pool);
    // LatestSpawnedRoom = GetWorld()->SpawnActor<AFF44RoomBase>(Chosen);

    LatestSpawnedRoom = GetWorld()->SpawnActor<AFF44RoomBase>(Pool[FMath::RandRange(0, Pool.Num() - 1)]);
    if (!LatestSpawnedRoom)
    {
        GetWorld()->GetTimerManager().SetTimer(SpawnNextHandle, this, &AFF44DungeonGenerator::SpawnNextRoom, RoomSpawnInterval, false);
        return;
    }

    LatestSpawnedRoom->SetActorLocation(SelectedExitPoint->GetComponentLocation());
    FRotator R = SelectedExitPoint->GetComponentRotation();
    R.Yaw += 90.f;
    LatestSpawnedRoom->SetActorRotation(R);

    const AFF44RoomBase* OwnerRoom = Cast<AFF44RoomBase>(SelectedExitPoint->GetOwner());
    const FName PrevTag = OwnerRoom ? OwnerRoom->RoomTypeTag : NAME_None;
    const FName NewTag = LatestSpawnedRoom->RoomTypeTag;

    if (PrevTag == FName("CrossRoad") && NewTag == FName("CrossRoad"))
    {
        LatestSpawnedRoom->Destroy();
        GetWorld()->GetTimerManager().SetTimer(SpawnNextHandle, this, &AFF44DungeonGenerator::SpawnNextRoom, RoomSpawnInterval, false);
        return;
    }

    if (RemoveOverlappingRooms())
    {
        LatestSpawnedRoom->Destroy();
        GetWorld()->GetTimerManager().SetTimer(SpawnNextHandle, this, &AFF44DungeonGenerator::SpawnNextRoom, RoomSpawnInterval, false);
        return;
    }

    TArray<USceneComponent*> NewExits;
    if (LatestSpawnedRoom->ExitPoints)
    {
        LatestSpawnedRoom->ExitPoints->GetChildrenComponents(false, NewExits);
    }

    const bool bHaveMoreToConnect = (Exits.Num() > 1) || (RoomsToSpawn > 1);
    if (NewExits.Num() == 0 && bHaveMoreToConnect)
    {
        LatestSpawnedRoom->Destroy();
        GetWorld()->GetTimerManager().SetTimer(SpawnNextHandle, this, &AFF44DungeonGenerator::SpawnNextRoom, RoomSpawnInterval, false);
        return;
    }

    Exits.Remove(SelectedExitPoint);
    Exits.Append(NewExits);

    if (RoomsToSpawn > 0)
    {
        RoomsToSpawn--;
    }

    TotalSpawned++;

    if (TotalSpawned >= MaxTotalRooms)
    {
        if (Exits.Num() > 0)
        {
            SealRemainingExits();
        }

        bDungeonCompleted = true;

        return;
    }

    if (RoomsToSpawn > 0 || Exits.Num() > 0)
    {
        GetWorld()->GetTimerManager().SetTimer(SpawnNextHandle, this, &AFF44DungeonGenerator::SpawnNextRoom, RoomSpawnInterval, false);
    }
    else
    {
        if (Exits.Num() > 0)
        {
            SealRemainingExits();
        }

        bDungeonCompleted = true;
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
    if (!ExitCapClass) return;

    for (USceneComponent* ExitComp : Exits)
    {
        if (!ExitComp) continue;

        const FVector Loc = ExitComp->GetComponentLocation();
        FRotator Rot = ExitComp->GetComponentRotation();
        Rot.Yaw += 90.f;

        if (ExitComp->ComponentHasTag(TEXT("Small")))
        {
            GetWorld()->SpawnActor<AActor>(SmallExitCapClass, Loc, Rot);
        }
        else
        {
            GetWorld()->SpawnActor<AActor>(ExitCapClass, Loc, Rot);
        }
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
        const int32 W = CDO ? FMath::Max(0, CDO->SpawnWeight) : 0; // 음수 방지
        TotalWeight += W;
    }

    if (TotalWeight <= 0)
    {
        return Pool[FMath::RandRange(0, Pool.Num() - 1)];
    }

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


