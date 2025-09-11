#include "Player/Abilities/GA_Player_PotionUse.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"

#include "Player/BasePlayer.h"
#include "Player/Data/PlayerTags.h"
#include "InventorySystem/InventoryComponent.h"

UGA_Player_PotionUse::UGA_Player_PotionUse()
{
    ActivationOwnedTags.AddTag(PlayerTags::State_Player_ItemUse_Potion);
}

void UGA_Player_PotionUse::CommitExecute(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo)
{
    if (UAnimInstance* AnimInst = OwnerPlayer->GetMesh()->GetAnimInstance())
    {
        AnimInst->OnPlayMontageNotifyBegin.AddDynamic(this, &UGA_Player_PotionUse::OnNotifyBegin);
        AnimInst->OnPlayMontageNotifyEnd.AddDynamic(this, &UGA_Player_PotionUse::OnNotifyEnd);

        UAbilityTask_PlayMontageAndWait* Task =
            UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
                this,
                NAME_None,
                ItemUseMontage,
                MontagePlayRate,
                NAME_None,
                false,
                1.0f
            );

        Task->OnCompleted.AddDynamic(this, &UGA_Player_PotionUse::OnCompleted);
        Task->OnBlendedIn.AddDynamic(this, &UGA_Player_PotionUse::OnBlendIn);
        Task->OnBlendOut.AddDynamic(this, &UGA_Player_PotionUse::OnBlendOut);
        Task->OnInterrupted.AddDynamic(this, &UGA_Player_PotionUse::OnInterrupted);
        Task->OnCancelled.AddDynamic(this, &UGA_Player_PotionUse::OnCancelled);
        Task->ReadyForActivation();
    }
}

void UGA_Player_PotionUse::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    if (Potion)
    {
        Potion->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        Potion->Destroy();
        Potion = nullptr;
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Player_PotionUse::OnNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& Payload)
{
    if (NotifyName == DrinkEndNotify)
    {
        // Ability를 가지고 있는지?
        if (UAbilitySystemComponent* ASC = OwnerPlayer->FindComponentByClass<UAbilitySystemComponent>())
        {
            // ASC 사용 가능
            if (HealEffectClass) {
                FGameplayEffectSpecHandle specHandle = ASC->MakeOutgoingSpec(HealEffectClass, 0.f, ASC->MakeEffectContext());
                FGameplayEffectSpec* spec = specHandle.Data.Get();

                OwnerPlayer->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*spec);
            }

            if (UWorld* World = GetWorld())
            {
                UGameplayStatics::PlaySound2D(World, EndDrinkSound);
                UGameplayStatics::PlaySound2D(World, HealSound);
            }

            UNiagaraComponent* NiagaraComp = 
                UNiagaraFunctionLibrary::SpawnSystemAttached(
                HealNiagara,                        
                OwnerPlayer->GetMesh(),             
                FName("Root"),                      
                FVector::ZeroVector,                
                FRotator::ZeroRotator,              
                EAttachLocation::KeepRelativeOffset,
                true,                               
                true,                               
                ENCPoolMethod::None,                
                true                                
            );

        }
    }
}

void UGA_Player_PotionUse::OnBlendIn()
{
	// 속도 조절
	OwnerPlayer->GetCharacterMovement()->MaxWalkSpeed = 100.f;

	// Potion 생성
	if (PotionClass)
	{
        if (UWorld* World = GetWorld())
        {
            Potion = World->SpawnActor<AActor>(
                PotionClass,
                FVector(0.f, 0.f, 0.f),
                FRotator::ZeroRotator
            );

            FAttachmentTransformRules rule = FAttachmentTransformRules(
                    EAttachmentRule::KeepRelative,
                    EAttachmentRule::KeepRelative,
                    EAttachmentRule::KeepRelative, true);

            Potion->AttachToComponent(OwnerPlayer->GetMesh(), rule, PotionSocketName);

            UGameplayStatics::PlaySound2D(World, StartDrinkSound);
        }
	}
}

bool UGA_Player_PotionUse::FindItem()
{
    if (IC->ConsumeItem(FName("potion")))
        return true;

    return false;
}
