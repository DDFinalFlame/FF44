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
    // === Ʃ�� ===
    UPROPERTY(EditDefaultsOnly, Category = "Assemble")
    FName PelvisBone = TEXT("Bip001-Pelvis"); // ������Ʈ ���̷��濡 �°�

    UPROPERTY(EditDefaultsOnly, Category = "Assemble")
    float FloorTraceDist = 2000.f;

    UPROPERTY(EditDefaultsOnly, Category = "Assemble")
    float StandSnapUpOffset = 2.f; // �ٴ� �� ��¦ ���

    UPROPERTY(EditDefaultsOnly, Category = "Assemble")
    float TickInterval = 0.02f; // ���� ƽ

    UPROPERTY(EditDefaultsOnly, Category = "Assemble")
    TEnumAsByte<ECollisionChannel> FloorTraceChannel = ECC_Visibility;

    UPROPERTY(EditDefaultsOnly, Category = "Assemble")
    float StepBlendTime = 0.35f;  // �� 1���� ���� �ð�

    UPROPERTY(EditDefaultsOnly, Category = "Assemble")
    TArray<FName> AssembleOrder = {
        // ����: ���ܡ��Ʈ���ü���ȡ�Ӹ�
        TEXT("Bip001-L-Foot"), TEXT("Bip001-L-Calf"), TEXT("Bip001-L-Thigh"),
        TEXT("Bip001-R-Foot"), TEXT("Bip001-R-Calf"), TEXT("Bip001-R-Thigh"),
        TEXT("Bip001-Pelvis"), TEXT("Bip001-Spine"), TEXT("Bip001-Spine2"),
        TEXT("Bip001-L-UpperArm"), TEXT("Bip001-L-Forearm"), TEXT("Bip001-L-Hand"),
        TEXT("Bip001-R-UpperArm"), TEXT("Bip001-R-Forearm"), TEXT("Bip001-R-Hand"),
        TEXT("Bip001-Neck"), TEXT("Bip001-Head")
    };

protected:
    // === ���� ���� ===
    TWeakObjectPtr<ACharacter> OwnerChar;
    int32 CurrentChainIndex = 0;
    FName CurrentBone;

    // === ���� ���� ===
    float BlendElapsed = 0.f;
    float BlendDuration = 0.f;
    FTimerHandle TH_BlendTick;

    // === ��ƿ ===
    USkeletalMeshComponent* GetMesh(ACharacter* Chr) const;
    UCapsuleComponent* GetCapsule(ACharacter* Chr) const;
    UCharacterMovementComponent* GetMove(ACharacter* Chr) const;
    AAIController* GetAI(ACharacter* Chr) const;

    static FHitResult TraceFloor(ACharacter* Chr, const FVector& From, float DownDist, ECollisionChannel Ch);

    // === ���� ��ƾ ===
    void Recover_Start(ACharacter* Chr);      // ĸ��/�޽� ���ĸ� �ϰ� ������ �����
    void AssembleStep();                      // ü�� �ϳ��� ����
    void StartSmoothBlend(const FName& StartBone, float Duration);
    void TickSmoothBlend();
    float EvalBlendAlpha(float T01);          // S-curve (0~1 -> 0~1)

    // ���� ���� ����
    void StandUpFix(ACharacter* Chr);
    void RestoreRotateFlags(ACharacter* Chr);

    // === GetUp(�Ͼ��) ������ Ʃ�� ===
    UPROPERTY(EditDefaultsOnly, Category = "Assemble")
    float GetUpTime = 1.6f;    // �ٴڿ��� ������ �Ͼ����� �ɸ��� �ð�

    // === GetUp ���� ===
    FVector GetUpStartLoc, GetUpTargetLoc;
    float   GetUpStartYaw = 0.f, GetUpTargetYaw = 0.f;
    float   GetUpElapsed = 0.f, GetUpDuration = 0.f;
    FTimerHandle TH_GetUpTick;

    // �Լ� ����
    void BeginGetUp(ACharacter* Chr);
    void TickGetUp();
    void FinishGetUp(ACharacter* Chr);

    // ���̾ư���
    // UGA_MonsterAssemble.h
    UPROPERTY(EditDefaultsOnly, Category = "Assemble|FX")
    UNiagaraSystem* NS_AssemblePop = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Assemble|FX")
    USoundBase* SFX_AssemblePop = nullptr;

    // === ���� ===
    FVector GetFeetOrPelvisLoc(ACharacter* Chr) const;
};