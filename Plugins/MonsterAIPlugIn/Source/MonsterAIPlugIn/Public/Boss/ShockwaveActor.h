#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShockwaveActor.generated.h"


class USphereComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class UAbilitySystemComponent;
class UGameplayEffect;

UCLASS()
class MONSTERAIPLUGIN_API AShockwaveActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AShockwaveActor();

    // === ������ ���� �Ķ���� ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shockwave|Config")
    float StartRadius = 50.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shockwave|Config")
    float MaxRadius = 2000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shockwave|Config")
    float ExpandSpeed = 2500.f; // cm/s

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shockwave|Damage")
    float ShockwaveDamage = 30.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shockwave|Damage")
    TSubclassOf<UGameplayEffect> GE_ShockwaveDamage;

    // Niagara
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shockwave|VFX")
    UNiagaraSystem* NiagaraSystemAsset;

    // �浹 ä��/���� Ŀ���͸������
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shockwave|Collision")
    TEnumAsByte<ECollisionChannel> PawnChannel = ECC_Pawn;

    // �����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shockwave|Debug")
    bool bDrawDebug = true;

    // ����(Instigator) ���ÿ� (���� ���ؽ�Ʈ�� ����)
    UFUNCTION(BlueprintCallable, Category = "Shockwave")
    void SetInstigatorActor(AActor* _Instigator);

    // === NS ���� ���� �����ϸ� �����ؼ� ������ ===
    UPROPERTY(EditAnywhere, Category = "VFX")
    float BaseVisualRadius = 120.f; // NS�� Scale=1 �� ������� ���� '�ݰ�'(cm)

    UPROPERTY(EditAnywhere, Category = "VFX")
    bool bUseScaleOverall = true;   // true: "Scale Overall" �� ��, false: X/Y ����

    // NS User Parameter �̸�(���� ������ ���̴� �̸��� '��Ȯ��' �����ؾ� �մϴ�)
    UPROPERTY(EditDefaultsOnly, Category = "VFX")
    FName NSParam_ScaleOverall = TEXT("Scale Overall");

    UPROPERTY(EditDefaultsOnly, Category = "VFX")
    FName NSParam_ScaleX = TEXT("Scale X Max");   // ���¿� ���ٸ� ����μ���
    UPROPERTY(EditDefaultsOnly, Category = "VFX")
    FName NSParam_ScaleY = TEXT("Scale Y Max");
    UPROPERTY(EditDefaultsOnly, Category = "VFX")
    FName NSParam_ScaleZ = TEXT("Scale Z Max");   // ���� Z�� 1.0 ����

    void UpdateNiagaraScale(); // ���� ����
protected:
    virtual void BeginPlay() override;
    virtual void Tick(float _DeltaSeconds) override;

    UFUNCTION()
    void OnSphereBeginOverlap(UPrimitiveComponent* _OverlappedComponent, AActor* _OtherActor,
        UPrimitiveComponent* _OtherComp, int32 _OtherBodyIndex,
        bool _bFromSweep, const FHitResult& _SweepResult);

private:
    void ApplyDamageOnce(AActor* _TargetActor);
    void UpdateRadius(float _DeltaSeconds);

private:
    UPROPERTY()
    USphereComponent* Sphere;

    UPROPERTY()
    UNiagaraComponent* NiagaraComp;

    float CurrentRadius = 0.f;

    // �̹� ���� �� ��� ���
    TSet<TWeakObjectPtr<AActor>> HitActors;

    // ���� ���ؽ�Ʈ��
    TWeakObjectPtr<AActor> InstigatorActor;
};
