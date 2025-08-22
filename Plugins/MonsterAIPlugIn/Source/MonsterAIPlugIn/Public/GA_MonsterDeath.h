// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_MonsterDeath.generated.h"

/**
 * 
 */
UCLASS()
class MONSTERAIPLUGIN_API UGA_MonsterDeath : public UGameplayAbility
{
    GENERATED_BODY()
public:
    UGA_MonsterDeath();

    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;
    
    UFUNCTION()
    void OnMontageEnded();
    
    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility, bool bWasCancelled) override;

private:
    // ������ ���̷�Ż �޽� �ؼ�(�⺻ Mesh�� null�̾ ��ü Ž��)
    static USkeletalMeshComponent* ResolveSkeletalMesh(ACharacter* Chr);

    // ���� ����(�̵�/AI/ĸ��/��Ÿ��/�ٸ� GA �ߴ�)
    void HardStopEverything(ACharacter* Chr, const FGameplayAbilityActorInfo* ActorInfo);

    // ���׵� ����(��������/����/�ִ� ����)
    void EnterRagdoll(ACharacter* Chr);
};
