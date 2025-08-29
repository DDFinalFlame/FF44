#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "EnemyWeaponCollisionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ENEMY_API UEnemyWeaponCollisionComponent : public UActorComponent
{
	GENERATED_BODY()

// Collider Data
protected:
	UPROPERTY(EditAnywhere)
	FName StartSocketName;

	UPROPERTY(EditAnywhere)
	FName EndSocketName;

	/* Sphere 반지름 **/
	UPROPERTY(EditAnywhere)
	float TraceRadius = 20.0f;

	/* Trace 대상 ObjectType **/
	UPROPERTY(EditAnywhere)
	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;

	/* 충돌 제외 Actors **/
	UPROPERTY(EditAnywhere)
	TArray<AActor*> IgnoredActors;

	/* 디버그 드로우 타입 **/
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType = EDrawDebugTrace::ForDuration;

protected:
	/* 무기 Mesh Component **/
	UPROPERTY()
	UPrimitiveComponent* WeaponMesh;

	UPROPERTY()
	TArray<AActor*> AlreadyHitActors;

	bool bIsCollisionEnabeld = false;

	/* 플레이어 충돌 시에만 True, 끄고 켤 때 무조건 False 초기화 **/
	UPROPERTY(VisibleAnywhere)
	bool bIsAttackSuccessful = false;

public:	
	UEnemyWeaponCollisionComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	/* Trace 충돌 확인 **/
	void CollisionTrace();
	/* 충돌 On/Off **/
	void ActivateCollision();
	void DeactivateCollision();

	/* 충돌 결과 Getter **/
	FORCEINLINE bool IsAttackSuccessful() const { return bIsAttackSuccessful; }

	///* 충돌 시 호출 함수 **/
	//UFUNCTION()
	//void OnColliderBeginOverlap(UPrimitiveComponent* OverlappedComponent, // 이벤트를 발생시킨 Collider
	//	AActor* OtherActor,                        // 충돌한 Actor
	//	UPrimitiveComponent* OtherComp,            // 충돌한 Actor의 Collider
	//	int32 OtherBodyIndex,                       // 충돌 Body Index
	//	bool bFromSweep,                            // Sweep 여부
	//	const FHitResult& SweepResult);

	/* 충돌 중복 확인 **/
	bool CanHitActor(AActor* HitActor);

	void SetWeaponMesh(UPrimitiveComponent* MeshComponent);

	void AddIgnoredActor(AActor* Actor);

	void RemoveIgnoredActor(AActor* Actor);
};
