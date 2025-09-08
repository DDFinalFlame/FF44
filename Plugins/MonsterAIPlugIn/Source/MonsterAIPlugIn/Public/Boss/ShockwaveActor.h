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

	/** 서버에서 스폰 직후 GA가 호출해 초기화 */
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

    // ====== 런타임 파라미터 ======
    UPROPERTY()
    UAbilitySystemComponent* SourceASC = nullptr;

    UPROPERTY()
    TSubclassOf<UGameplayEffect> DamageGE;

    UPROPERTY()
    AActor* SourceActor = nullptr; // 보스 액터(Instigator)

    float Damage = 0.f;
    float MaxRadius = 0.f;
    float ExpandSpeed = 1000.f; // 초당 확산 속도
    float CurrentRadius = 10.f;

    // 중복 타격 방지
    TSet<TWeakObjectPtr<AActor>> HitActors;

    // 팀/필터가 있다면 태그나 인터페이스 등으로 교체 가능
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

    // 서버 전용 보조
    bool IsServerAuth() const { return HasAuthority(); }


    // 나이아가라
    UPROPERTY(EditDefaultsOnly, Category = "Shockwave|VFX")
    UNiagaraSystem* NiagaraTemplate = nullptr;

    // VFX 기준 반경(스케일 1.0일 때 화면상 '반지름'이 몇 UU인지)
    // 링 메시/머티리얼에 따라 다르므로 한 번만 보정하면 됩니다.
    UPROPERTY(EditDefaultsOnly, Category = "Shockwave|VFX", meta = (ClampMin = "1.0"))
    float VisualBaseRadius = 50.f;

    UPROPERTY() UNiagaraComponent* NiagaraComp = nullptr;

};
