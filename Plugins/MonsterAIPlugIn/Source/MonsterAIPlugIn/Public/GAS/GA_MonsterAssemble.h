#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_MonsterAssemble.generated.h"

class ACharacter;
class USkeletalMeshComponent;
class UCapsuleComponent;
class UCharacterMovementComponent;
class AAIController;
class UNiagaraSystem;
class USoundBase;

UCLASS()
class UGA_MonsterAssemble : public UGameplayAbility
{
    GENERATED_BODY()
public:
    UGA_MonsterAssemble();

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo ActInfo,
        const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo ActInfo,
        bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
    // === 튜닝 ===
    UPROPERTY(EditDefaultsOnly, Category = "Assemble")
    FName PelvisBone = TEXT("Bip001-Pelvis"); // 프로젝트 스켈레톤에 맞게

    UPROPERTY(EditDefaultsOnly, Category = "Assemble")
    float FloorTraceDist = 2000.f;

    UPROPERTY(EditDefaultsOnly, Category = "Assemble")
    float StandSnapUpOffset = 2.f; // 바닥 위 살짝 띄움

    UPROPERTY(EditDefaultsOnly, Category = "Assemble")
    float TickInterval = 0.02f; // 블렌드 틱

    UPROPERTY(EditDefaultsOnly, Category = "Assemble")
    TEnumAsByte<ECollisionChannel> FloorTraceChannel = ECC_Visibility;

    UPROPERTY(EditDefaultsOnly, Category = "Assemble")
    float StepBlendTime = 0.35f;  // 본 1개당 블렌드 시간

    UPROPERTY(EditDefaultsOnly, Category = "Assemble")
    TArray<FName> AssembleOrder = {
        // 예시: 말단→루트→상체→팔→머리
        TEXT("Bip001-L-Foot"), TEXT("Bip001-L-Calf"), TEXT("Bip001-L-Thigh"),
        TEXT("Bip001-R-Foot"), TEXT("Bip001-R-Calf"), TEXT("Bip001-R-Thigh"),
        TEXT("Bip001-Pelvis"), TEXT("Bip001-Spine"), TEXT("Bip001-Spine2"),
        TEXT("Bip001-L-UpperArm"), TEXT("Bip001-L-Forearm"), TEXT("Bip001-L-Hand"),
        TEXT("Bip001-R-UpperArm"), TEXT("Bip001-R-Forearm"), TEXT("Bip001-R-Hand"),
        TEXT("Bip001-Neck"), TEXT("Bip001-Head")
    };

protected:
    // === 진행 상태 ===
    TWeakObjectPtr<ACharacter> OwnerChar;
    int32 CurrentChainIndex = 0;
    FName CurrentBone;

    // === 블렌드 상태 ===
    float BlendElapsed = 0.f;
    float BlendDuration = 0.f;
    FTimerHandle TH_BlendTick;

    // === 유틸 ===
    USkeletalMeshComponent* GetMesh(ACharacter* Chr) const;
    UCapsuleComponent* GetCapsule(ACharacter* Chr) const;
    UCharacterMovementComponent* GetMove(ACharacter* Chr) const;
    AAIController* GetAI(ACharacter* Chr) const;

    static FHitResult TraceFloor(ACharacter* Chr, const FVector& From, float DownDist, ECollisionChannel Ch);

    // === 메인 루틴 ===
    void Recover_Start(ACharacter* Chr);      // 캡슐/메시 정렬만 하고 물리는 살려둠
    void AssembleStep();                      // 체인 하나씩 조립
    void StartSmoothBlend(const FName& StartBone, float Duration);
    void TickSmoothBlend();
    float EvalBlendAlpha(float T01);          // S-curve (0~1 -> 0~1)

    // 최종 안전 정렬
    void StandUpFix(ACharacter* Chr);
    void RestoreRotateFlags(ACharacter* Chr);

    // === GetUp(일어나기) 페이즈 튜닝 ===
    UPROPERTY(EditDefaultsOnly, Category = "Assemble")
    float GetUpTime = 1.6f;    // 바닥에서 완전히 일어서기까지 걸리는 시간

    // === GetUp 상태 ===
    FVector GetUpStartLoc, GetUpTargetLoc;
    float   GetUpStartYaw = 0.f, GetUpTargetYaw = 0.f;
    float   GetUpElapsed = 0.f, GetUpDuration = 0.f;
    FTimerHandle TH_GetUpTick;

    // 함수 선언
    void BeginGetUp(ACharacter* Chr);
    void TickGetUp();
    void FinishGetUp(ACharacter* Chr);

    // 나이아가라
    // UGA_MonsterAssemble.h
    UPROPERTY(EditDefaultsOnly, Category = "Assemble|FX")
    UNiagaraSystem* NS_AssemblePop = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Assemble|FX")
    USoundBase* SFX_AssemblePop = nullptr;

    // === 헬퍼 ===
    FVector GetFeetOrPelvisLoc(ACharacter* Chr) const;
};