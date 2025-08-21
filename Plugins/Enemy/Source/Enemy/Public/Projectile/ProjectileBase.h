#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ProjectileBase.generated.h"

UCLASS()
class ENEMY_API AProjectileBase : public AActor
{
	GENERATED_BODY()

protected:
	/* �޽� **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UStaticMeshComponent* MeshComponent;

	/* �浹ü **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* CapsuleComponent;

	/* �̵� **/
	UPROPERTY(VisibleAnywhere, Category = "Movement")
	UProjectileMovementComponent* ProjectileMovementComponent;
	
public:	
	AProjectileBase();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

public:
	/* �߻� �������� �ӵ� �ʱ�ȭ **/
	void FireInDirection(const FVector& ShootDirection);
};
