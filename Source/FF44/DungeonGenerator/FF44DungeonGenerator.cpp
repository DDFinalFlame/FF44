// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator/FF44DungeonGenerator.h"
#include "DungeonGenerator/DungeonBase/FF44StarterRoom.h"
#include "DungeonGenerator/DungeonBase/FF44RoomBase.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"

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

    MonsterSpawnMarkers.Empty();
    InteractableSpawnMarkers.Empty();

    CollectMonsterMarkersFromRoom(OutStarter);
    CollectInteractableMarkersFromRoom(OutStarter);
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

    // 1) 출구/풀 선택
    SelectedExitPoint = Exits[FMath::RandRange(0, Exits.Num() - 1)];
    const bool bSmallExit = SelectedExitPoint->ComponentHasTag(TEXT("Small"));
    const TArray<TSubclassOf<AFF44RoomBase>>& Pool = bSmallExit ? SmallRoomsToBeSpawned : RoomsToBeSpawned;

    if (Pool.Num() == 0)
    {
        GetWorld()->GetTimerManager().SetTimer(SpawnNextHandle, this, &AFF44DungeonGenerator::SpawnNextRoom, RoomSpawnInterval, false);
        return;
    }

    // 2) 방 스폰
    // (가중치 사용 시 아래 두 줄 교체)
    // TSubclassOf<AFF44RoomBase> Chosen = PickWeightedRoom(Pool);
    // LatestSpawnedRoom = GetWorld()->SpawnActor<AFF44RoomBase>(Chosen);
    LatestSpawnedRoom = GetWorld()->SpawnActor<AFF44RoomBase>(Pool[FMath::RandRange(0, Pool.Num() - 1)]);
    if (!LatestSpawnedRoom)
    {
        GetWorld()->GetTimerManager().SetTimer(SpawnNextHandle, this, &AFF44DungeonGenerator::SpawnNextRoom, RoomSpawnInterval, false);
        return;
    }

    // 3) 위치/회전(Exit 화살표 보정)
    LatestSpawnedRoom->SetActorLocation(SelectedExitPoint->GetComponentLocation());
    FRotator R = SelectedExitPoint->GetComponentRotation();
    R.Yaw += 90.f;
    LatestSpawnedRoom->SetActorRotation(R);

    // 4) 연속 태그 제한 (CrossRoad 연속 금지)
    const AFF44RoomBase* OwnerRoom = Cast<AFF44RoomBase>(SelectedExitPoint->GetOwner());
    const FName PrevTag = OwnerRoom ? OwnerRoom->RoomTypeTag : NAME_None;
    const FName NewTag = LatestSpawnedRoom->RoomTypeTag;
    if (PrevTag == FName("CrossRoad") && NewTag == FName("CrossRoad"))
    {
        LatestSpawnedRoom->Destroy();
        GetWorld()->GetTimerManager().SetTimer(SpawnNextHandle, this, &AFF44DungeonGenerator::SpawnNextRoom, RoomSpawnInterval, false);
        return;
    }

    // 5) 겹침 검사
    if (RemoveOverlappingRooms())
    {
        LatestSpawnedRoom->Destroy();
        GetWorld()->GetTimerManager().SetTimer(SpawnNextHandle, this, &AFF44DungeonGenerator::SpawnNextRoom, RoomSpawnInterval, false);
        return;
    }

    // 6) 새 출구 채집
    TArray<USceneComponent*> NewExits;
    if (LatestSpawnedRoom->ExitPoints)
    {
        LatestSpawnedRoom->ExitPoints->GetChildrenComponents(false, NewExits);
    }

    // 7) 막다른길 방지(아직 붙일 게 남았는데 새 출구가 전혀 없으면 경로 폐기)
    const bool bHaveMoreToConnect = (Exits.Num() > 1) || (RoomsToSpawn > 1);
    if (NewExits.Num() == 0 && bHaveMoreToConnect)
    {
        LatestSpawnedRoom->Destroy();
        GetWorld()->GetTimerManager().SetTimer(SpawnNextHandle, this, &AFF44DungeonGenerator::SpawnNextRoom, RoomSpawnInterval, false);
        return;
    }

    // 8) 출구 갱신 & 마커 수집
    Exits.Remove(SelectedExitPoint);
    Exits.Append(NewExits);

    CollectMonsterMarkersFromRoom(LatestSpawnedRoom);
    CollectInteractableMarkersFromRoom(LatestSpawnedRoom);

    // 9) 카운트/종료 판정
    if (RoomsToSpawn > 0)
    {
        RoomsToSpawn--;
    }
    TotalSpawned++;

    if (TotalSpawned >= MaxTotalRooms)
    {
        PlaceFloorGoalAndFinish();
        return;
    }

    if (RoomsToSpawn > 0 || Exits.Num() > 0)
    {
        GetWorld()->GetTimerManager().SetTimer(SpawnNextHandle, this, &AFF44DungeonGenerator::SpawnNextRoom, RoomSpawnInterval, false);
    }
    else
    {
        PlaceFloorGoalAndFinish();
    }
}

bool AFF44DungeonGenerator::RemoveOverlappingRooms()
{
    return IsRoomOverlapping(LatestSpawnedRoom);
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

bool AFF44DungeonGenerator::IsRoomOverlapping(AFF44RoomBase* Room) const
{
    if (!Room || !Room->OverlapFolder) return false;

    TArray<USceneComponent*> Boxes;
    Room->OverlapFolder->GetChildrenComponents(false, Boxes);

    for (USceneComponent* C : Boxes)
    {
        if (UBoxComponent* Box = Cast<UBoxComponent>(C))
        {
            const_cast<UBoxComponent*>(Box)->UpdateOverlaps();

            TArray<UPrimitiveComponent*> Hits;
            Box->GetOverlappingComponents(Hits);

            for (UPrimitiveComponent* Hit : Hits)
            {
                if (!Hit) continue;
                if (Hit->GetOwner() == Room) continue;
                return true;
            }
        }
    }
    return false;
}

USceneComponent* AFF44DungeonGenerator::SelectGoalExit() const
{
    if (Exits.Num() == 0) return nullptr;

    TArray<USceneComponent*> NonSmall;
    for (USceneComponent* E : Exits)
    {
        if (E && !E->ComponentHasTag(TEXT("Small")))
        {
            NonSmall.Add(E);
        }
    }
    if (NonSmall.Num() > 0)
    {
        return NonSmall[FMath::RandRange(0, NonSmall.Num() - 1)];
    }
    return Exits[FMath::RandRange(0, Exits.Num() - 1)];
}

void AFF44DungeonGenerator::PlaceFloorGoalAndFinish()
{
    TSubclassOf<AFF44RoomBase> GoalCls = bIsBossFloor ? BossRoomClass : PortalRoomClass;

    bool bPlaced = false;

    if (*GoalCls && Exits.Num() > 0)
    {
        TArray<int32> Idx;
        Idx.Reserve(Exits.Num());
        for (int32 i = 0; i < Exits.Num(); ++i) Idx.Add(i);
        for (int32 i = 0; i < Idx.Num(); ++i) { Idx.Swap(i, FMath::RandRange(i, Idx.Num() - 1)); }

        for (int32 k = 0; k < Idx.Num(); ++k)
        {
            const int32 ExitIndex = Idx[k];
            USceneComponent* ExitComp = Exits[ExitIndex];
            if (!ExitComp) continue;

            const FVector Loc = ExitComp->GetComponentLocation();
            FRotator Rot = ExitComp->GetComponentRotation();
            Rot.Yaw += 90.f;

            AFF44RoomBase* Goal = GetWorld()->SpawnActor<AFF44RoomBase>(GoalCls, Loc, Rot);
            if (!Goal) continue;

            if (IsRoomOverlapping(Goal))
            {
                Goal->Destroy();
                continue;
            }

            Exits.RemoveAt(ExitIndex);

            bPlaced = true;
            break;
        }
    }

    if (Exits.Num() > 0)
    {
        SealRemainingExits();
    }
    Exits.Empty();

    bDungeonCompleted = true;
    OnDungeonComplete.Broadcast();
}

void AFF44DungeonGenerator::CollectMonsterMarkersFromRoom(const AFF44RoomBase* Room)
{
    if (!Room || !Room->MonsterSpawnPoints) return;

    TArray<USceneComponent*> Points;
    Room->MonsterSpawnPoints->GetChildrenComponents(false, Points);

    for (USceneComponent* C : Points)
    {
        UArrowComponent* Arrow = Cast<UArrowComponent>(C);
        if (!Arrow) continue;

        FMonsterSpawnInfo Info;
        Info.Transform = Arrow->GetComponentTransform();
        Info.Tag = (Arrow->ComponentTags.Num() > 0) ? Arrow->ComponentTags[0] : NAME_None;

        MonsterSpawnMarkers.Add(Info);
    }
}

void AFF44DungeonGenerator::CollectInteractableMarkersFromRoom(const AFF44RoomBase* Room)
{
    if (!Room || !Room->InteractableSpawnPoints) return;

    TArray<USceneComponent*> Points;
    Room->InteractableSpawnPoints->GetChildrenComponents(false, Points);

    for (USceneComponent* C : Points)
    {
        UArrowComponent* Arrow = Cast<UArrowComponent>(C);
        if (!Arrow) continue;

        FInteractableSpawnInfo Info;
        Info.Transform = Arrow->GetComponentTransform();
        Info.Tag = (Arrow->ComponentTags.Num() > 0) ? Arrow->ComponentTags[0] : NAME_None;

        InteractableSpawnMarkers.Add(Info);
    }
}

TSubclassOf<AFF44RoomBase> AFF44DungeonGenerator::PickWeightedRoom(const TArray<TSubclassOf<AFF44RoomBase>>& Pool) const
{
    if (Pool.Num() == 0) return nullptr;

    int32 TotalWeight = 0;
    for (const TSubclassOf<AFF44RoomBase>& Cls : Pool)
    {
        if (!*Cls) continue;
        const AFF44RoomBase* CDO = Cls->GetDefaultObject<AFF44RoomBase>();
        const int32 W = CDO ? FMath::Max(0, CDO->SpawnWeight) : 0;
        TotalWeight += W;
    }

    if (TotalWeight <= 0)
    {
        return Pool[FMath::RandRange(0, Pool.Num() - 1)];
    }

    const int32 Pick = FMath::RandRange(1, TotalWeight);
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


