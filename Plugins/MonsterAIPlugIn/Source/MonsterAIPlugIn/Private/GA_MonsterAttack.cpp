#include "GA_MonsterAttack.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemComponent.h"
#include "MonsterCharacter.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"


UGA_MonsterAttack::UGA_MonsterAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 메인 태그 부여
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Monster.Attack")));

}

void UGA_MonsterAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    AMonsterCharacter* Monster = Cast<AMonsterCharacter>(ActorInfo->AvatarActor.Get());
    if (Monster && Monster->AttackMontage)
    {
        // 섹션 이름 배열 생성
        TArray<FName> AttackSections = { FName("Attack1"), FName("Attack2"), FName("Attack3") };

        // 랜덤으로 섹션 선택
        int32 RandomIndex = FMath::RandRange(0, AttackSections.Num() - 1);
        FName SelectedSection = AttackSections[RandomIndex];

        // Task 생성 시 섹션 이름 지정
        UAbilityTask_PlayMontageAndWait* Task =
            UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
                this,
                NAME_None,
                Monster->AttackMontage,
                1.f,
                SelectedSection,  // 여기가 핵심
                false
            );

        if (Task)
        {
            Task->OnCompleted.AddDynamic(this, &UGA_MonsterAttack::OnMontageCompleted);
            Task->OnInterrupted.AddDynamic(this, &UGA_MonsterAttack::OnMontageCancelled);
            Task->OnCancelled.AddDynamic(this, &UGA_MonsterAttack::OnMontageCancelled);
            Task->Activate();
        }
    }
    else
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false); // 몽타주 없으면 바로 종료
    }
}

void UGA_MonsterAttack::OnMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_MonsterAttack::OnMontageCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}