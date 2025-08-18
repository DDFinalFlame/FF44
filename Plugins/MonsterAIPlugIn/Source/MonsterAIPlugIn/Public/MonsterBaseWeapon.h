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

	// ����â On/Off (AnimNotify���� ȣ��)
	UFUNCTION(BlueprintCallable)
	virtual void BeginAttackWindow();
	UFUNCTION(BlueprintCallable)
	virtual void EndAttackWindow();

	// �ܺο��� ������ ���� ����(Def/DT��)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float Damage = 10.f;

protected:
    UPROPERTY()
    AMonsterCharacter* OwnerMonster = nullptr;

    // ��Ʈ�ڽ���(Į ���� ��, ���� �и� � ����)
    UPROPERTY()
    TArray<UPrimitiveComponent*> Hitboxes;

    // �� ���� �� �ߺ� Ÿ�� ����
    TSet<TWeakObjectPtr<AActor>> HitActorsThisSwing;

    bool bActive = false;

    // �Ļ����� �ڽ� ���� �� ����� ���
    void RegisterHitbox(UPrimitiveComponent* Comp);

    // ���� ������ �ڵ鷯
    UFUNCTION()
    void OnHitboxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    // ���� ����
    virtual void ApplyHit(AActor* Victim, const FHitResult& Hit);

    // ���� ���� Ȱ��ȭ
    virtual void SetHitboxEnable(bool bEnable);

    UFUNCTION()
    void OnOwnerDestroyed(AActor* DestroyedActor);

};
