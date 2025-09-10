// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/FF44SpikeTrap.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Character.h"
#include "Player/BasePlayer.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Sound/SoundBase.h"

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

    DamageArea = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageArea"));
    DamageArea->SetupAttachment(SpikeMesh);
    DamageArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DamageArea->SetCollisionResponseToAllChannels(ECR_Ignore);
    DamageArea->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    DamageArea->SetGenerateOverlapEvents(true);
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

    if (DamageArea)
    {
        DamageArea->OnComponentBeginOverlap.AddDynamic(this, &AFF44SpikeTrap::OnDamageBegin);
        DamageArea->OnComponentEndOverlap.AddDynamic(this, &AFF44SpikeTrap::OnDamageEnd);
    }

	bArmed = true;
    SetActive(true);
}

void AFF44SpikeTrap::OnTriggerBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& Sweep)
{
    if (!OtherActor || !bArmed) return;

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
    if (!OtherActor || !bArmed) return;

    if (Cast<APawn>(OtherActor))
    {
        OverlapCount = FMath::Max(0, OverlapCount - 1);
    }
}

bool AFF44SpikeTrap::CanInteract_Implementation(AActor* Interactor) const
{
    return bArmed;
}

void AFF44SpikeTrap::Interact_Implementation(AActor* Interactor)
{
    if (!bArmed) return;

    bArmed = false;
    bCycling = false;

    GetWorldTimerManager().ClearTimer(DelayHandle);
    GetWorldTimerManager().ClearTimer(HoldHandle);
    GetWorldTimerManager().ClearTimer(CooldownHandle);

    SpikeMesh->SetRelativeLocation(SpikeDownLocal);

    if (DamageArea)
    {
        DamageArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        DamageArea->SetGenerateOverlapEvents(false);
    }
    if (TriggerBox)
    {
        TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    SetPromptVisible(false);
}

void AFF44SpikeTrap::StartCycle()
{
    if (!bArmed || bCycling) return;

    bCycling = true;

    GetWorldTimerManager().SetTimer(DelayHandle, this, &AFF44SpikeTrap::RaiseSpikes, TriggerDelay, false);
}

void AFF44SpikeTrap::RaiseSpikes()
{
    if (!bArmed)
    {
        bCycling = false;
        return;
    }

    if (RaiseSFX)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            RaiseSFX,
            SpikeMesh ? SpikeMesh->GetComponentLocation() : GetActorLocation()
        );
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
    if (!bArmed)
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

    if (!bArmed && OverlapCount > 0)
    {
        StartCycle();
    }
}

void AFF44SpikeTrap::OnRaised()
{
    BP_OnSpikesRaised();
    OnSpikesRaised.Broadcast();

    GetWorldTimerManager().SetTimer(HoldHandle, this, &AFF44SpikeTrap::RetractSpikes, HoldTime, false);
}

void AFF44SpikeTrap::OnRetracted()
{
    GetWorldTimerManager().SetTimer(CooldownHandle, this, &AFF44SpikeTrap::EndCooldown, CooldownTime, false);
}

void AFF44SpikeTrap::OnDamageBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	if (!bArmed || !bActive || !OtherActor) return;

    UE_LOG(LogTemp, Warning, TEXT("Spike Trap Damage Begin"));

    if (auto Player = Cast<ABasePlayer>(OtherActor))
    {
        FGameplayAbilitySpec spec = DamageAbility;
        auto playerAbility = Player->GetAbilitySystemComponent();

        if (DamageAbility)
        {
            DamageAbilityHandle = playerAbility->GiveAbilityAndActivateOnce(spec);
        }
    }
}

void AFF44SpikeTrap::OnDamageEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!bArmed || !bActive || !OtherActor) return;

    UE_LOG(LogTemp, Warning, TEXT("Spike Trap Damage End"));

    if (auto Player = Cast<ABasePlayer>(OtherActor))
    {
        auto playerAbility = Player->GetAbilitySystemComponent();
        playerAbility->ClearAbility(DamageAbilityHandle);
    }
}
