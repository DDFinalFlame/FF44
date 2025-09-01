
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Chaos/ChaosEngineInterface.h"           // FChaosPhysicsCollisionInfo
#include "Field/FieldSystemComponent.h"
#include "Field/FieldSystemObjects.h"
#include "FallingRockActor.generated.h"


class UBoxComponent;
class UNiagaraSystem;
class USoundBase;
class UGameplayEffect;
class UAbilitySystemComponent;

UCLASS()
class MONSTERAIPLUGIN_API AFallingRockActor : public AActor
{
	GENERATED_BODY()

public:
	AFallingRockActor();

protected:
	/** 프랙처용: GeometryCollection 루트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UGeometryCollectionComponent* GeoComp;

	/** 판정용 충돌 박스 (메시와 별도로) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* HitBox;

	/** (선택) 필드 적용용 FieldSystem 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UFieldSystemComponent* FieldSystem;

public:
	// ====== 기존 공개 프로퍼티 유지 ======
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock")
	float InitialDownSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock")
	bool bDestroyOnGroundHit = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock")
	float DestroyDelayOnGround = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock")
	float LifeSeconds = 6.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock|FX")
	UNiagaraSystem* ImpactFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock|FX")
	USoundBase* ImpactSound = nullptr;

	// === 데미지 / GAS ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rock|Damage")
	TSubclassOf<UGameplayEffect> GE_Damage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rock|Damage", meta = (Categories = "Data"))
	FGameplayTag ByCallerDamageTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock|Damage")
	float DamageMagnitude = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock|Damage")
	bool bOncePerActor = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock|Damage")
	bool bIgnoreOwnerAndInstigatorTeam = true;

	UFUNCTION(BlueprintCallable)
	void SetDamageInstigator(AActor* InInstigator);

	/** 지면 충돌 시 프랙처 강제 유도에 사용할 반경/강도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock|Fracture")
	float FractureRadius = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FallingRock|Fracture")
	float FractureStrength = 50000.f;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDebugHitBox = true;

	UPROPERTY(EditAnywhere, Category = "Rock")
	bool bStopOnFirstBreak = false;


	//디버깅용
	UPROPERTY(EditAnywhere, Category = "Rock|Debug")
	bool bShowPredictedImpact = true;

	UPROPERTY(EditAnywhere, Category = "Rock|Debug", meta = (ClampMin = "1000", ClampMax = "200000"))
	float PredictMaxDownTrace = 50000.f; // 최대 하향 추적 거리

	UPROPERTY(EditAnywhere, Category = "Rock|Debug")
	float PredictMarkerLife = 6.f; // 디버그 마커 수명(초)

	UPROPERTY(EditAnywhere, Category = "Rock|Debug")
	FColor PredictMarkerColor = FColor::Orange;

	UPROPERTY(EditAnywhere, Category = "Rock|Debug")
	float PredictMarkerSize = 30.f; // 마커 크기(박스/스피어 반지름 등)


protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds);

	/** Chaos 물리 충돌 이벤트(GeoComp) */
	UFUNCTION()
	void OnChaosCollision(const FChaosPhysicsCollisionInfo& CollisionInfo);

	/** GeometryCollection 브레이크 이벤트(파편 분리) */
	UFUNCTION()
	void OnChaosBreak(const FChaosBreakEvent& BreakEvent);

	UFUNCTION()
	void OnHitBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	void ApplyDamageTo(AActor* Target, const FHitResult& OptionalHit);
	UAbilitySystemComponent* GetASCFromActor(AActor* Actor) const;

	bool ShouldIgnore(AActor* OtherActor) const;

	/** 필드(스트레인)로 프랙처를 유도하는 헬퍼 */
	void ApplyFractureFieldAt(const FVector& Center);

	bool bHitBoxActive = false;
	void SetHitBoxActive(bool bActive);

	/** 착지/정지 처리(공통) */
	void HandleLanded(const FVector& At);

	/** 바닥 스윕으로 착지 보강 */
	bool SweepGroundHit(FHitResult& OutHit) const;

private:
	TSet<TWeakObjectPtr<AActor>> AlreadyHitSet;
	TWeakObjectPtr<AActor> DamageInstigator;
	bool bHasLanded = false;

private:
	bool HitBoxTouchesGround(FVector& OutHitPoint) const;

private:
	bool PredictImpactPoint(FVector& OutPoint, FHitResult& OutHit) const;
	void DrawImpactMarker(const FVector& At, const FVector& Normal) const;
};