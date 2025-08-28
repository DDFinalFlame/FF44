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
        // ASC�� ���� �ʱ�ȭ ���̸� ���� ƽ�� ��õ�
        GetWorldTimerManager().SetTimerForNextTick(this, &ABossCharacter::ActivatePhaseWatcherOnce);
        return;
    }

    // ASC�� ActorInfo�� �� ������ ���ε��ƴ��� Ȯ�� (InitAbilityActorInfo ���Ŀ��� ��)
    if (AbilitySystemComponent->GetAvatarActor() != this)
    {
        GetWorldTimerManager().SetTimerForNextTick(this, &ABossCharacter::ActivatePhaseWatcherOnce);
        return;
    }

    if (!bPhaseWatcherActivated && Phase1AbilityClass)
    {
        // ���� DA�� GA_BossPhase1�� �̹� Granted �Ǿ� �־�� ��
        AbilitySystemComponent->TryActivateAbilityByClass(Phase1AbilityClass);
        bPhaseWatcherActivated = true;
    }
}