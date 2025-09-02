// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/FF44BladeTrap.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Player/BasePlayer.h"
#include "AbilitySystemBlueprintLibrary.h"

AFF44BladeTrap::AFF44BladeTrap()
{
    PrimaryActorTick.bCanEverTick = true;

    Pivot = CreateDefaultSubobject<USceneComponent>(TEXT("Pivot"));
    RootComponent = Pivot;

    BladeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BladeMesh"));
    BladeMesh->SetupAttachment(Pivot);
    BladeMesh->SetCollisionProfileName(TEXT("NoCollision"));

    DamageArea = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageArea"));
    DamageArea->SetupAttachment(BladeMesh);
    DamageArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DamageArea->SetCollisionResponseToAllChannels(ECR_Ignore);
    DamageArea->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    DamageArea->SetGenerateOverlapEvents(true);
}

void AFF44BladeTrap::BeginPlay()
{
    Super::BeginPlay();

    if (DamageArea)
    {
        DamageArea->OnComponentBeginOverlap.AddDynamic(this, &AFF44BladeTrap::OnDamageBegin);
        DamageArea->OnComponentEndOverlap.AddDynamic(this, &AFF44BladeTrap::OnDamageEnd);
    }

	SetActive(true);
}

void AFF44BladeTrap::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    RunningTime += DeltaSeconds * SwingSpeed;
    float Angle = FMath::Sin(RunningTime) * SwingAmplitude;

    Pivot->SetRelativeRotation(FRotator(Angle, 0.f, 0.f));
}

void AFF44BladeTrap::OnDamageBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	if (!bArmed || !bActive || !OtherActor) return;

    UE_LOG(LogTemp, Warning, TEXT("Blade Trap Damage Begin"));

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

void AFF44BladeTrap::OnDamageEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!bArmed || !bActive || !OtherActor) return;

    UE_LOG(LogTemp, Warning, TEXT("Blade Trap Damage End"));

    if (auto Player = Cast<ABasePlayer>(OtherActor))
    {
        auto playerAbility = Player->GetAbilitySystemComponent();
        playerAbility->ClearAbility(DamageAbilityHandle);
    }
}
