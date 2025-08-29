// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss/BossCharacter.h"
#include "AIController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MonsterTags.h"
#include "Data/staticName.h"


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

    if (!bPhaseWatcherActivated)
    {
        if (Phase1AbilityClass)
        {
            AbilitySystemComponent->TryActivateAbilityByClass(Phase1AbilityClass);
        }

        if (Phase2AbilityClass)
        {
            AbilitySystemComponent->TryActivateAbilityByClass(Phase2AbilityClass);
        }
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


void ABossCharacter::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);

    // 디버그
    UE_LOG(LogTemp, Warning, TEXT("Boss Landed!"));

    // Phase2 GA에서 기다리는 착지 이벤트 송신
    FGameplayEventData Data;
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        this, MonsterTags::Event_Boss_Land, Data);
}