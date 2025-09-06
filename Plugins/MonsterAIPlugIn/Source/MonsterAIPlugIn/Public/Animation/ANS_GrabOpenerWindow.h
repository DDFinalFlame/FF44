// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_GrabOpenerWindow.generated.h"

class ABossMeleeWeapon;

UCLASS()
class MONSTERAIPLUGIN_API UANS_GrabOpenerWindow : public UAnimNotifyState
{
	GENERATED_BODY()
public:
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;

    // 필요하면 같이 일반 히트박스도 켜고싶을 때 사용
    UPROPERTY(EditAnywhere, Category = "Grab")
    bool bAlsoEnableNormalHitbox = false;
};
