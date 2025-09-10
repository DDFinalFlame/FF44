#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "BossCinematicTrigger.generated.h"

class UBoxComponent;
class UBillboardComponent;
class UGameplayAbility;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCinematicTriggered, AActor*, OtherActor);

UCLASS()
class MONSTERAIPLUGIN_API ABossCinematicTrigger : public AActor
{
    GENERATED_BODY()

public:
    ABossCinematicTrigger();

    UPROPERTY(BlueprintAssignable, Category = "Cinematic")
    FOnCinematicTriggered OnCinematicTriggered;

    UFUNCTION(BlueprintCallable, Category = "Cinematic")
    void PlayCinematicNow();  

    UFUNCTION(BlueprintImplementableEvent, Category = "Cinematic")
    void BP_OnCinematicTriggered(AActor* OtherActor); 

protected:
    UPROPERTY(VisibleAnywhere, Category = "Trigger")
    UBoxComponent* Box;

    UPROPERTY(VisibleAnywhere, Category = "Trigger")
    UBillboardComponent* Marker;

    // �ڵ� ���� �ɼ�
    UPROPERTY(EditAnywhere, Category = "Boss|AutoResolve")
    bool bAutoResolveBoss = true;                 // �ڵ����� ���� ã��

    UPROPERTY(EditAnywhere, Category = "Boss|AutoResolve")
    FName BossActorTag = TEXT("Rampage");            // ���� BP�� Actor Tags�� "Boss" �޾Ƶα�

    UPROPERTY(EditAnywhere, Category = "Boss|AutoResolve")
    TSubclassOf<AActor> BossClassHint;            // ���� Ŭ���� ��Ʈ(������ �켱)

    UPROPERTY(EditAnywhere, Category = "Boss|AutoResolve", meta = (ClampMin = "0.0"))
    float ResolveSearchRadius = 0.f;              // 0=������, >0 �̸� �ݰ� ��������

    // NEW: ����������Ŭ �������̵�
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // NEW: ���� �Լ� & �ڵ�
    void TryResolveBoss();                        // ���� ���忡�� ���� 1ȸ �˻�
    void OnActorSpawned(AActor* NewActor);        // ���� �����Ǵ� ���� ����

    FDelegateHandle ActorSpawnedHandle;           // ���� ��������Ʈ ������

    // Ʈ���Ű� ������ ��� ����
    UPROPERTY(EditAnywhere, Category = "Boss")
    AActor* TargetBoss;

    // (����1) ���� ��Ʈ�� ��Ÿ��
    UPROPERTY(EditAnywhere, Category = "Boss")
    UAnimMontage* BossIntroMontage;

    // (����2) ���� ��Ʈ�� �ɷ�(GAS)
    UPROPERTY(EditAnywhere, Category = "Boss|GAS")
    TSubclassOf<UGameplayAbility> IntroAbilityClass;

    // (����3) �������� ���� �����÷��� �̺�Ʈ �±� (�ɷ¿��� WaitGameplayEvent�� �ޱ�)
    UPROPERTY(EditAnywhere, Category = "Boss|GAS")
    FGameplayTag BossIntroEventTag;

    // ������ Ű�� �� ��(���� ��ȯ). Ű �̸��� ���(FBlackboardKeySelector ���ʿ�)
    UPROPERTY(EditAnywhere, Category = "Boss|AI")
    FName BossStateBBKey = TEXT("BossState");

    UPROPERTY(EditAnywhere, Category = "Boss|AI")
    FName InBattleBBKey = TEXT("In Battle");

    UPROPERTY(EditAnywhere, Category = "Boss|AI", meta = (ClampMin = "0", ClampMax = "255"))
    uint8 BossStateValue = 0; // ��: IntroCutscene ��

    // �� ���� �۵�
    UPROPERTY(EditAnywhere, Category = "Trigger")
    bool bOneShot = true;

    // �÷��̾ ����
    UPROPERTY(EditAnywhere, Category = "Trigger")
    bool bOnlyPlayer = true;

    // �ߵ� �� ��Ȱ��ȭ/�ı�
    UPROPERTY(EditAnywhere, Category = "Trigger")
    bool bDestroyAfterTrigger = true;

    // ���� ����
    bool bTriggered;

    UFUNCTION()
    void OnBeginOverlap(UPrimitiveComponent* _OverlappedComp, AActor* _OtherActor,
        UPrimitiveComponent* _OtherComp, int32 _OtherBodyIndex,
        bool _bFromSweep, const FHitResult& _SweepResult);


    void TriggerOnce(AActor* _OtherActor);
    void PlayBossIntroMontage();
    void ActivateBossIntroAbility();
    void SendBossIntroEvent();
    void SetBossStateBB();
};