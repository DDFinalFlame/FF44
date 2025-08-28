// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss/BossCharacter.h"


ABossCharacter::ABossCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ABossCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        ActivatePhaseWatcherOnce();
    }

}

void ABossCharacter::ActivatePhaseWatcherOnce()
{
    if (!HasAuthority()) return;
    if (!AbilitySystemComponent)
    {
        // ASC가 아직 초기화 전이면 다음 틱에 재시도
        GetWorldTimerManager().SetTimerForNextTick(this, &ABossCharacter::ActivatePhaseWatcherOnce);
        return;
    }

    // ASC의 ActorInfo가 이 보스에 바인딩됐는지 확인 (InitAbilityActorInfo 이후여야 함)
    if (AbilitySystemComponent->GetAvatarActor() != this)
    {
        GetWorldTimerManager().SetTimerForNextTick(this, &ABossCharacter::ActivatePhaseWatcherOnce);
        return;
    }

    if (!bPhaseWatcherActivated && Phase1AbilityClass)
    {
        // 보스 DA에 GA_BossPhase1이 이미 Granted 되어 있어야 함
        AbilitySystemComponent->TryActivateAbilityByClass(Phase1AbilityClass);
        bPhaseWatcherActivated = true;
    }
}