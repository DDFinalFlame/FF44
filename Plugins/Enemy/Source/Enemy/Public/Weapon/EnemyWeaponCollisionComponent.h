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

	/* Sphere ������ **/
	UPROPERTY(EditAnywhere)
	float TraceRadius = 20.0f;

	/* Trace ��� ObjectType **/
	UPROPERTY(EditAnywhere)
	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;

	/* �浹 ���� Actors **/
	UPROPERTY(EditAnywhere)
	TArray<AActor*> IgnoredActors;

	/* ����� ��ο� Ÿ�� **/
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType = EDrawDebugTrace::ForDuration;

protected:
	/* ���� Mesh Component **/
	UPROPERTY()
	UPrimitiveComponent* WeaponMesh;

	UPROPERTY()
	TArray<AActor*> AlreadyHitActors;

	bool bIsCollisionEnabeld = false;

	/* �÷��̾� �浹 �ÿ��� True, ���� �� �� ������ False �ʱ�ȭ **/
	UPROPERTY(VisibleAnywhere)
	bool bIsAttackSuccessful = false;

public:	
	UEnemyWeaponCollisionComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	/* Trace �浹 Ȯ�� **/
	void CollisionTrace();
	/* �浹 On/Off **/
	void ActivateCollision();
	void DeactivateCollision();

	/* �浹 ��� Getter **/
	FORCEINLINE bool IsAttackSuccessful() const { return bIsAttackSuccessful; }

	///* �浹 �� ȣ�� �Լ� **/
	//UFUNCTION()
	//void OnColliderBeginOverlap(UPrimitiveComponent* OverlappedComponent, // �̺�Ʈ�� �߻���Ų Collider
	//	AActor* OtherActor,                        // �浹�� Actor
	//	UPrimitiveComponent* OtherComp,            // �浹�� Actor�� Collider
	//	int32 OtherBodyIndex,                       // �浹 Body Index
	//	bool bFromSweep,                            // Sweep ����
	//	const FHitResult& SweepResult);

	/* �浹 �ߺ� Ȯ�� **/
	bool CanHitActor(AActor* HitActor);

	void SetWeaponMesh(UPrimitiveComponent* MeshComponent);

	void AddIgnoredActor(AActor* Actor);

	void RemoveIgnoredActor(AActor* Actor);
};
