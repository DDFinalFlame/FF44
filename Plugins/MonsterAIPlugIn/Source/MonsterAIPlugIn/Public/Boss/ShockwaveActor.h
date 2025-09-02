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

    // === 에디터 노출 파라미터 ===
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

    // 충돌 채널/응답 커스터마이즈용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shockwave|Collision")
    TEnumAsByte<ECollisionChannel> PawnChannel = ECC_Pawn;

    // 디버그
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shockwave|Debug")
    bool bDrawDebug = true;

    // 보스(Instigator) 세팅용 (피해 컨텍스트에 넣음)
    UFUNCTION(BlueprintCallable, Category = "Shockwave")
    void SetInstigatorActor(AActor* _Instigator);

    // === NS 수정 없이 스케일만 조절해서 퍼지게 ===
    UPROPERTY(EditAnywhere, Category = "VFX")
    float BaseVisualRadius = 120.f; // NS를 Scale=1 로 재생했을 때의 '반경'(cm)

    UPROPERTY(EditAnywhere, Category = "VFX")
    bool bUseScaleOverall = true;   // true: "Scale Overall" 한 방, false: X/Y 개별

    // NS User Parameter 이름(에셋 좌측에 보이는 이름과 '정확히' 동일해야 합니다)
    UPROPERTY(EditDefaultsOnly, Category = "VFX")
    FName NSParam_ScaleOverall = TEXT("Scale Overall");

    UPROPERTY(EditDefaultsOnly, Category = "VFX")
    FName NSParam_ScaleX = TEXT("Scale X Max");   // 에셋에 없다면 비워두세요
    UPROPERTY(EditDefaultsOnly, Category = "VFX")
    FName NSParam_ScaleY = TEXT("Scale Y Max");
    UPROPERTY(EditDefaultsOnly, Category = "VFX")
    FName NSParam_ScaleZ = TEXT("Scale Z Max");   // 보통 Z는 1.0 유지

    void UpdateNiagaraScale(); // 내부 헬퍼
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

    // 이미 피해 준 대상 목록
    TSet<TWeakObjectPtr<AActor>> HitActors;

    // 피해 컨텍스트용
    TWeakObjectPtr<AActor> InstigatorActor;
};
