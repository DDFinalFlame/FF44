#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_BossAttack_Swing1.generated.h"

UCLASS()
class MONSTERAIPLUGIN_API UGA_BossAttack_Swing1 : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UGA_BossAttack_Swing1();

protected:
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    UFUNCTION()
    void OnMontageCompleted();

    UFUNCTION()
    void OnMontageCancelled();

private:
    // ���߿� DataTable�̳� MonsterDefinition���� ���� ����
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
    TSoftObjectPtr<UAnimMontage> AttackMontage;
};