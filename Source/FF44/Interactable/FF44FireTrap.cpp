// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/FF44FireTrap.h"
#include "Components/BoxComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Player/BasePlayer.h"
#include "AbilitySystemBlueprintLibrary.h"

AFF44FireTrap::AFF44FireTrap()
{
    DamageArea = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageArea"));
    DamageArea->SetupAttachment(RootComponent);
    DamageArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    DamageArea->SetCollisionResponseToAllChannels(ECR_Ignore);
    DamageArea->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    DamageArea->SetGenerateOverlapEvents(false);

    FireEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FireEffect"));
    FireEffect->SetupAttachment(RootComponent);
    FireEffect->bAutoActivate = false;

    bUseTriggerBox = true;
}

void AFF44FireTrap::BeginPlay()
{
    Super::BeginPlay();

    if (DamageArea)
    {
        DamageArea->OnComponentBeginOverlap.AddDynamic(this, &AFF44FireTrap::OnDamageBegin);
        DamageArea->OnComponentEndOverlap.AddDynamic(this, &AFF44FireTrap::OnDamageEnd);
    }

    SetActive(false);
}

void AFF44FireTrap::SetActive(bool bInActive)
{
    Super::SetActive(bInActive);

    if (FireEffect)
    {
        if (bInActive) FireEffect->Activate(true);
        else           FireEffect->Deactivate();
    }

    if (DamageArea)
    {
        DamageArea->SetGenerateOverlapEvents(bInActive);
        DamageArea->SetCollisionEnabled(bInActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
    }
}

void AFF44FireTrap::OnDamageBegin(UPrimitiveComponent* Overlapped, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIdx, bool bFromSweep, const FHitResult& Sweep)
{
    if (!bArmed || !bActive || !OtherActor) return;

  //  if (auto Player = Cast<ABasePlayer>(OtherActor))
  //  {
  //      auto playerAbility = Player->GetAbilitySystemComponent();
  //      auto EffectContext = playerAbility->MakeEffectContext();
		//EffectContext.AddSourceObject(this);

  //      auto spec = playerAbility->MakeOutgoingSpec(GameplayEffect, 1, EffectContext);
  //      
  //      if (spec.Data.IsValid())
  //      {
		//	playerAbility->ApplyGameplayEffectSpecToSelf(*spec.Data.Get());
  //          
  //          FGameplayEventData Payload;
  //          Payload.EventTag = FGameplayTag::RequestGameplayTag("Event.Player.Hit");
  //          Payload.Instigator = this;
  //          Payload.Target = OtherActor;
  //          UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OtherActor, Payload.EventTag, Payload);
  //      }
  //  }

	UE_LOG(LogTemp, Warning, TEXT("Fire Trap Damage Begin"));

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

void AFF44FireTrap::OnDamageEnd(UPrimitiveComponent* Overlapped, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIdx)
{
    if (!bArmed || !bActive || !OtherActor) return;

    UE_LOG(LogTemp, Warning, TEXT("Fire Trap Damage End"));

    if (auto Player = Cast<ABasePlayer>(OtherActor))
    {
        auto playerAbility = Player->GetAbilitySystemComponent();
        playerAbility->ClearAbility(DamageAbilityHandle);
	}
}
