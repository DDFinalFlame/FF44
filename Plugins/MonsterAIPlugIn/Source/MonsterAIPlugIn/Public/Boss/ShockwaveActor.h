// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "ShockwaveActor.generated.h"

class USphereComponent;
class UGameplayEffect;
class UAbilitySystemComponent;

UCLASS()
class MONSTERAIPLUGIN_API AShockwaveActor : public AActor
{
	GENERATED_BODY()

public:
	AShockwaveActor();

	/** �������� ���� ���� GA�� ȣ���� �ʱ�ȭ */
	UFUNCTION(BlueprintCallable, Category = "Shockwave")
	void Initialize(UAbilitySystemComponent* InSourceASC,
		TSubclassOf<UGameplayEffect> InDamageGE,
		float InDamage,
		float InMaxRadius,
		float InExpandSpeed,
		AActor* InSourceActor);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere, Category = "Shockwave")
    USphereComponent* Sphere;

    // ====== ��Ÿ�� �Ķ���� ======
    UPROPERTY()
    UAbilitySystemComponent* SourceASC = nullptr;

    UPROPERTY()
    TSubclassOf<UGameplayEffect> DamageGE;

    UPROPERTY()
    AActor* SourceActor = nullptr; // ���� ����(Instigator)

    float Damage = 0.f;
    float MaxRadius = 0.f;
    float ExpandSpeed = 1000.f; // �ʴ� Ȯ�� �ӵ�
    float CurrentRadius = 10.f;

    // �ߺ� Ÿ�� ����
    TSet<TWeakObjectPtr<AActor>> HitActors;

    // ��/���Ͱ� �ִٸ� �±׳� �������̽� ������ ��ü ����
    UPROPERTY(EditDefaultsOnly, Category = "Shockwave|Collision")
    TEnumAsByte<ECollisionChannel> PawnChannel = ECC_Pawn;

    UFUNCTION()
    void OnSphereBeginOverlap(UPrimitiveComponent* Overlapped,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    void ApplyDamageToActor(AActor* TargetActor);

    // ���� ���� ����
    bool IsServerAuth() const { return HasAuthority(); }


    // ���̾ư���
    UPROPERTY(EditDefaultsOnly, Category = "Shockwave|VFX")
    UNiagaraSystem* NiagaraTemplate = nullptr;

    // VFX ���� �ݰ�(������ 1.0�� �� ȭ��� '������'�� �� UU����)
    // �� �޽�/��Ƽ���� ���� �ٸ��Ƿ� �� ���� �����ϸ� �˴ϴ�.
    UPROPERTY(EditDefaultsOnly, Category = "Shockwave|VFX", meta = (ClampMin = "1.0"))
    float VisualBaseRadius = 50.f;

    UPROPERTY() UNiagaraComponent* NiagaraComp = nullptr;

};
