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
	//보스 소켓
	UPROPERTY(EditAnywhere, Category = "Grab|Parent(Boss)")
	FName BossSocketName = TEXT("hand_r");

	// 플레이어 소켓
	UPROPERTY(EditAnywhere, Category = "Grab|Child(Victim)")
	FName VictimAttachSocketName;// = TEXT("");


	UPROPERTY(EditAnywhere, Category = "Victim")
	FName BB_TargetActorKey = TEXT("TargetActor");

	// 혹시 플레이어 타입을 제한하고 싶을 때
	UPROPERTY(EditAnywhere, Category = "Victim")
	TSubclassOf<ACharacter> VictimClassFilter;

	UPROPERTY(EditAnywhere, Category = "Grab")
	bool bSnapToBossSocket = true;

	UPROPERTY(EditAnywhere, Category = "Throw")
	float LaunchSpeed = 1200.f;

	UPROPERTY(EditAnywhere, Category = "Throw")
	float LaunchUpward = 300.f;

	// 시작할 때,
	UPROPERTY(EditAnywhere, Category = "Victim")
	FGameplayTag VictimStartEventTag = MonsterTags::Event_Player_Grab_AniStart;

	// 끝날 때 
	UPROPERTY(EditAnywhere, Category = "Victim")
	FGameplayTag VictimEndEventTag = MonsterTags::Event_Player_Grab_AniEnd;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

private:
	ACharacter* ResolveVictimFromBlackboard(ACharacter* Boss) const;
	// 희생자 소켓을 보스 소켓에 정렬하도록 희생자 루트의 월드 트랜스폼을 계산
	void AlignVictimRootToBossSocket(ACharacter* Boss, ACharacter* Victim) const;
};
