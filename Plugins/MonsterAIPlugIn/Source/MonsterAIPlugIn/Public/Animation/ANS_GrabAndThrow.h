// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "MonsterTags.h"
#include "ANS_GrabAndThrow.generated.h"

class ACharacter;
UCLASS(meta = (DisplayName = "Grab And Throw (Paired)"))
class MONSTERAIPLUGIN_API UANS_GrabAndThrow : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	//���� ����
	UPROPERTY(EditAnywhere, Category = "Grab|Parent(Boss)")
	FName BossSocketName = TEXT("hand_r");

	// �÷��̾� ����
	UPROPERTY(EditAnywhere, Category = "Grab|Child(Victim)")
	FName VictimAttachSocketName;// = TEXT("");


	UPROPERTY(EditAnywhere, Category = "Victim")
	FName BB_TargetActorKey = TEXT("TargetActor");

	// Ȥ�� �÷��̾� Ÿ���� �����ϰ� ���� ��
	UPROPERTY(EditAnywhere, Category = "Victim")
	TSubclassOf<ACharacter> VictimClassFilter;

	UPROPERTY(EditAnywhere, Category = "Grab")
	bool bSnapToBossSocket = true;

	UPROPERTY(EditAnywhere, Category = "Throw")
	float LaunchSpeed = 1200.f;

	UPROPERTY(EditAnywhere, Category = "Throw")
	float LaunchUpward = 300.f;

	// ������ ��,
	UPROPERTY(EditAnywhere, Category = "Victim")
	FGameplayTag VictimStartEventTag = MonsterTags::Event_Player_Grab_AniStart;

	// ���� �� 
	UPROPERTY(EditAnywhere, Category = "Victim")
	FGameplayTag VictimEndEventTag = MonsterTags::Event_Player_Grab_AniEnd;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

private:
	ACharacter* ResolveVictimFromBlackboard(ACharacter* Boss) const;
	// ����� ������ ���� ���Ͽ� �����ϵ��� ����� ��Ʈ�� ���� Ʈ�������� ���
	void AlignVictimRootToBossSocket(ACharacter* Boss, ACharacter* Victim) const;
};
