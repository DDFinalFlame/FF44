#include "GA_MonsterAttack.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemComponent.h"
#include "MonsterCharacter.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"


UGA_MonsterAttack::UGA_MonsterAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// ���� �±� �ο�
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
        // ���� �̸� �迭 ����
        TArray<FName> AttackSections = { FName("Attack1"), FName("Attack2"), FName("Attack3") };

        // �������� ���� ����
        int32 RandomIndex = FMath::RandRange(0, AttackSections.Num() - 1);
        FName SelectedSection = AttackSections[RandomIndex];

        // Task ���� �� ���� �̸� ����
        UAbilityTask_PlayMontageAndWait* Task =
            UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
                this,
                NAME_None,
                Monster->AttackMontage,
                1.f,
                SelectedSection,  // ���Ⱑ �ٽ�
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
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false); // ��Ÿ�� ������ �ٷ� ����
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