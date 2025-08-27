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
    // 안전한 스켈레탈 메시 해석(기본 Mesh가 null이어도 대체 탐색)
    static USkeletalMeshComponent* ResolveSkeletalMesh(ACharacter* Chr);

    // 공통 정리(이동/AI/캡슐/몽타주/다른 GA 중단)
    void HardStopEverything(ACharacter* Chr, const FGameplayAbilityActorInfo* ActorInfo);

    // 래그돌 진입(프로파일/물리/애님 정지)
    void EnterRagdoll(ACharacter* Chr);
};
