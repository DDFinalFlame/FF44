// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/FF44FireTrap.h"
#include "Components/BoxComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

AFF44FireTrap::AFF44FireTrap()
{
    PrimaryActorTick.bCanEverTick = true;

    DamageArea = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageArea"));
    DamageArea->SetupAttachment(RootComponent);
    DamageArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DamageArea->SetCollisionResponseToAllChannels(ECR_Ignore);
    DamageArea->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    DamageArea->SetGenerateOverlapEvents(true);

    FXRoot = CreateDefaultSubobject<USceneComponent>(TEXT("FXRoot"));
    FXRoot->SetupAttachment(RootComponent);
    FXRoot->SetUsingAbsoluteRotation(true);
    FXRoot->SetUsingAbsoluteScale(true);

    FireEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FireEffect"));
    FireEffect->SetupAttachment(FXRoot);
    FireEffect->bAutoActivate = false;
}

void AFF44FireTrap::BeginPlay()
{
    Super::BeginPlay();

    if (DamageArea)
    {
        DamageArea->OnComponentBeginOverlap.AddDynamic(this, &AFF44FireTrap::OnDamageAreaBegin);
        DamageArea->OnComponentEndOverlap.AddDynamic(this, &AFF44FireTrap::OnDamageAreaEnd);
    }

    if (bArmed)
    {
        SetActive(true);
        StartCycle();
    }
    else
    {
        SetActive(false);
    }
}

void AFF44FireTrap::StartCycle()
{
    if (!bArmed) { SetActive(false); return; }

    GetWorldTimerManager().ClearTimer(CycleTimerHandle);
    const float Delay = bActive ? ActiveTime : InactiveTime;
    GetWorldTimerManager().SetTimer(CycleTimerHandle, this, &AFF44FireTrap::OnCycleTick, Delay, false);
}

void AFF44FireTrap::OnCycleTick()
{
    if (!bArmed) { SetActive(false); return; }

    SetActive(!bActive);
    StartCycle();
}

void AFF44FireTrap::SetActive(bool bInActive)
{
    bActive = bInActive;

    if (DamageArea)
    {
        DamageArea->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryOnly
            : ECollisionEnabled::NoCollision);
    }

    if (FireEffect)
    {
        if (bActive)
        {
            FireEffect->Activate(true);
            FireEffect->SetActive(true, true);
            FireEffect->SetVisibility(true);
        }
        else
        {
            FireEffect->DeactivateSystem();
            FireEffect->SetActive(false);
            FireEffect->SetVisibility(false);
        }
    }
}

void AFF44FireTrap::OnDamageAreaBegin(UPrimitiveComponent* Overlapped, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIdx, bool bFromSweep,
    const FHitResult& Sweep)
{
    if (!bArmed || !bActive) return;
    if (!OtherActor || OtherActor == this) return;

    const float Now = GetWorld()->GetTimeSeconds();
    if (float* Last = LastHitTime.Find(OtherActor))
    {
        if ((Now - *Last) < PerActorDamageCooldown) return;
    }

    UGameplayStatics::ApplyDamage(
        OtherActor, Damage, nullptr, this,
        DamageTypeClass ? *DamageTypeClass : UDamageType::StaticClass());

    LastHitTime.Add(OtherActor, Now);
}

void AFF44FireTrap::OnDamageAreaEnd(UPrimitiveComponent* Overlapped, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIdx)
{
    LastHitTime.Remove(OtherActor);
}

void AFF44FireTrap::Interact_Implementation(AActor* Interactor)
{
    if (!bArmed) return;

    SetArmed(false);

    GetWorldTimerManager().ClearTimer(CycleTimerHandle);
    SetActive(false);

    // 필요 시 여기서 BP 이벤트 호출 가능 (ex. BP_OnDisarmed())
}