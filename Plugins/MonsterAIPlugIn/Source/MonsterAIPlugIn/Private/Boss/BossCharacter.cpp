// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss/BossCharacter.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
static const FName KEY_BossState = TEXT("BossState");

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


void ABossCharacter::SetBossState_EBB(uint8 NewState)
{
    if (!HasAuthority()) return;
    AAIController* AICon = Cast<AAIController>(GetController());
    if (!AICon) return;
    UBlackboardComponent* BB = AICon->GetBlackboardComponent();
    if (!BB) return;

    BB->SetValueAsEnum(KEY_BossState, NewState);
}

void ABossCharacter::SetBossState_Name(FName BBKeyName, uint8 NewState)
{
    if (!HasAuthority()) return;
    AAIController* AICon = Cast<AAIController>(GetController());
    if (!AICon) return;
    if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
    {
        BB->SetValueAsEnum(BBKeyName, NewState);
    }
}


void ABossCharacter::SetBlackboardTargetActor(FName BBKeyName, AActor* NewTarget)
{
   // if (!HasAuthority()) return;

    AAIController* AICon = Cast<AAIController>(GetController());
    if (!AICon) return;

    if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
    {
        BB->SetValueAsObject(BBKeyName, NewTarget);
    }
}