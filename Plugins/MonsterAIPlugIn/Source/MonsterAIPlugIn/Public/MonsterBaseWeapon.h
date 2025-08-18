// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterBaseWeapon.generated.h"

class UPrimitiveComponent;
class UBoxComponent;
class USkeletalMeshComponent;
class UAbilitySystemComponent;
class AMonsterCharacter;

UCLASS()
class MONSTERAIPLUGIN_API AMonsterBaseWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AMonsterBaseWeapon();

	void Init(AMonsterCharacter* InOwner, USkeletalMeshComponent* AttachTo, FName Socket);

	// 공격창 On/Off (AnimNotify에서 호출)
	UFUNCTION(BlueprintCallable)
	virtual void BeginAttackWindow();
	UFUNCTION(BlueprintCallable)
	virtual void EndAttackWindow();

	// 외부에서 데미지 조정 가능(Def/DT로)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float Damage = 10.f;

protected:
    UPROPERTY()
    AMonsterCharacter* OwnerMonster = nullptr;

    // 히트박스들(칼 여러 개, 궤적 분리 등도 대응)
    UPROPERTY()
    TArray<UPrimitiveComponent*> Hitboxes;

    // 한 스윙 중 중복 타격 방지
    TSet<TWeakObjectPtr<AActor>> HitActorsThisSwing;

    bool bActive = false;

    // 파생에서 박스 생성 후 여기로 등록
    void RegisterHitbox(UPrimitiveComponent* Comp);

    // 공통 오버랩 핸들러
    UFUNCTION()
    void OnHitboxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    // 실제 적용
    virtual void ApplyHit(AActor* Victim, const FHitResult& Hit);

    // 서버 전용 활성화
    virtual void SetHitboxEnable(bool bEnable);

    UFUNCTION()
    void OnOwnerDestroyed(AActor* DestroyedActor);

};
