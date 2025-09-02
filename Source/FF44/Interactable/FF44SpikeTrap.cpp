// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/FF44SpikeTrap.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Character.h"

AFF44SpikeTrap::AFF44SpikeTrap()
{
    PrimaryActorTick.bCanEverTick = true;

    SpikeRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SpikeRoot"));
    RootComponent = SpikeRoot;

    if (Mesh)
    {
        Mesh->SetupAttachment(SpikeRoot);
        Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Mesh->SetHiddenInGame(true);
    }

    BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
    BaseMesh->SetupAttachment(RootComponent);
    BaseMesh->SetCollisionProfileName(TEXT("BlockAll"));

    SpikeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpikeMesh"));
    SpikeMesh->SetupAttachment(SpikeRoot);
    SpikeMesh->SetCollisionProfileName(TEXT("NoCollision"));

    DamageZone = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageZone"));
    DamageZone->SetupAttachment(SpikeMesh);
    DamageZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    DamageZone->SetCollisionResponseToAllChannels(ECR_Ignore);
    DamageZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    DamageZone->SetGenerateOverlapEvents(true);
}

void AFF44SpikeTrap::BeginPlay()
{
    Super::BeginPlay();

    SpikeDownLocal = SpikeMesh->GetRelativeLocation();
    SpikeUpLocal = SpikeDownLocal + FVector(0, 0, SpikeRaiseDistance);

    if (TriggerBox)
    {
        TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AFF44SpikeTrap::OnTriggerBegin);
        TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AFF44SpikeTrap::OnTriggerEnd);
    }
}

void AFF44SpikeTrap::OnTriggerBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& Sweep)
{
    if (!OtherActor || bDisarmed) return;

    if (Cast<APawn>(OtherActor))
    {
        OverlapCount++;

        if (!bCycling)
        {
            StartCycle();
        }
    }
}

void AFF44SpikeTrap::OnTriggerEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor || bDisarmed) return;

    if (Cast<APawn>(OtherActor))
    {
        OverlapCount = FMath::Max(0, OverlapCount - 1);
    }
}

bool AFF44SpikeTrap::CanInteract_Implementation(AActor* Interactor) const
{
    return !bDisarmed;
}

void AFF44SpikeTrap::Interact_Implementation(AActor* Interactor)
{
    if (bDisarmed) return;

    bDisarmed = true;
    bCycling = false;

    GetWorldTimerManager().ClearTimer(DelayHandle);
    GetWorldTimerManager().ClearTimer(HoldHandle);
    GetWorldTimerManager().ClearTimer(CooldownHandle);

    SpikeRoot->SetRelativeLocation(SpikeDownLocal);

    if (DamageZone)
    {
        DamageZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    if (TriggerBox)
    {
        TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    SetPromptVisible(false);
}

void AFF44SpikeTrap::StartCycle()
{
    if (bDisarmed || bCycling) return;

    bCycling = true;

    GetWorldTimerManager().SetTimer(DelayHandle, this, &AFF44SpikeTrap::RaiseSpikes, TriggerDelay, false);
}

void AFF44SpikeTrap::RaiseSpikes()
{
    if (bDisarmed)
    {
        bCycling = false;
        return;
    }

    FLatentActionInfo LatentInfo;
    LatentInfo.CallbackTarget = this;
    LatentInfo.ExecutionFunction = FName("OnRaised");
    LatentInfo.Linkage = 0;
    LatentInfo.UUID = 1001;

    UKismetSystemLibrary::MoveComponentTo(
        SpikeMesh,
        SpikeUpLocal,
        SpikeMesh->GetRelativeRotation(),
        bEaseOut,
        bEaseIn,
        RaiseTime,
        false,
        EMoveComponentAction::Move,
        LatentInfo
    );
}

void AFF44SpikeTrap::RetractSpikes()
{
    if (bDisarmed)
    {
        bCycling = false;
        return;
    }

    FLatentActionInfo LatentInfo;
    LatentInfo.CallbackTarget = this;
    LatentInfo.ExecutionFunction = FName("OnRetracted");
    LatentInfo.Linkage = 0;
    LatentInfo.UUID = 1002;

    UKismetSystemLibrary::MoveComponentTo(
        SpikeMesh,
        SpikeDownLocal,
        SpikeMesh->GetRelativeRotation(),
        bEaseOut,
        bEaseIn,
        LowerTime,
        false,
        EMoveComponentAction::Move,
        LatentInfo
    );
}

void AFF44SpikeTrap::EndCooldown()
{
    bCycling = false;

    if (!bDisarmed && OverlapCount > 0)
    {
        StartCycle();
    }
}

void AFF44SpikeTrap::ApplyDamageOnce()
{
    if (!DamageZone) return;

    TArray<AActor*> Overlapping;
    DamageZone->GetOverlappingActors(Overlapping, APawn::StaticClass());

    for (AActor* A : Overlapping)
    {
        if (!A) continue;
        UGameplayStatics::ApplyDamage(A, Damage, nullptr, this, DamageType ? *DamageType : nullptr);
    }

    // 필요하면 소리/파티클 재생 지점
}

void AFF44SpikeTrap::OnRaised()
{
    ApplyDamageOnce();

    GetWorldTimerManager().SetTimer(HoldHandle, this, &AFF44SpikeTrap::RetractSpikes, HoldTime, false);
}

void AFF44SpikeTrap::OnRetracted()
{
    GetWorldTimerManager().SetTimer(CooldownHandle, this, &AFF44SpikeTrap::EndCooldown, CooldownTime, false);
}
