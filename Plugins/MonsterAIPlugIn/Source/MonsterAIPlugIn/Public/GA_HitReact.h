#pragma once
#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayEffectTypes.h"
#include "GA_HitReact.generated.h"

UCLASS()
class MONSTERAIPLUGIN_API UGA_HitReact : public UGameplayAbility
{
    GENERATED_BODY()
public:
    UGA_HitReact();
    UPROPERTY(EditDefaultsOnly, Category = "HitReact")
    TSubclassOf<class UGameplayEffect> HitReactEffectClass;

protected:

    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
        const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility, bool bWasCancelled) override;

    // 델리게이트용 핸들러(여기에 바인딩)
    UFUNCTION()
    void OnMontageCompleted();

    UFUNCTION()
    void OnMontageInterrupted();

    UFUNCTION()
    void OnMontageCancelled();

private:
    class UAnimMontage* GetMonsterHitMontage(const FGameplayAbilityActorInfo* Info) const;

    FActiveGameplayEffectHandle ActiveGE;



};