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

    // 자동 보스 옵션
    UPROPERTY(EditAnywhere, Category = "Boss|AutoResolve")
    bool bAutoResolveBoss = true;                 // 자동으로 보스 찾기

    UPROPERTY(EditAnywhere, Category = "Boss|AutoResolve")
    FName BossActorTag = TEXT("Rampage");            // 보스 BP의 Actor Tags에 "Boss" 달아두기

    UPROPERTY(EditAnywhere, Category = "Boss|AutoResolve")
    TSubclassOf<AActor> BossClassHint;            // 보스 클래스 힌트(있으면 우선)

    UPROPERTY(EditAnywhere, Category = "Boss|AutoResolve", meta = (ClampMin = "0.0"))
    float ResolveSearchRadius = 0.f;              // 0=무제한, >0 이면 반경 내에서만

    // NEW: 라이프사이클 오버라이드
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // NEW: 내부 함수 & 핸들
    void TryResolveBoss();                        // 현재 월드에서 보스 1회 검색
    void OnActorSpawned(AActor* NewActor);        // 이후 스폰되는 보스 감지

    FDelegateHandle ActorSpawnedHandle;           // 스폰 델리게이트 해제용

    // 트리거가 조작할 대상 보스
    UPROPERTY(EditAnywhere, Category = "Boss")
    AActor* TargetBoss;

    // (선택1) 보스 인트로 몽타주
    UPROPERTY(EditAnywhere, Category = "Boss")
    UAnimMontage* BossIntroMontage;

    // (선택2) 보스 인트로 능력(GAS)
    UPROPERTY(EditAnywhere, Category = "Boss|GAS")
    TSubclassOf<UGameplayAbility> IntroAbilityClass;

    // (선택3) 보스에게 보낼 게임플레이 이벤트 태그 (능력에서 WaitGameplayEvent로 받기)
    UPROPERTY(EditAnywhere, Category = "Boss|GAS")
    FGameplayTag BossIntroEventTag;

    // 블랙보드 키와 쓸 값(상태 전환). 키 이름만 사용(FBlackboardKeySelector 불필요)
    UPROPERTY(EditAnywhere, Category = "Boss|AI")
    FName BossStateBBKey = TEXT("BossState");

    UPROPERTY(EditAnywhere, Category = "Boss|AI")
    FName InBattleBBKey = TEXT("In Battle");

    UPROPERTY(EditAnywhere, Category = "Boss|AI", meta = (ClampMin = "0", ClampMax = "255"))
    uint8 BossStateValue = 0; // 예: IntroCutscene 등

    // 한 번만 작동
    UPROPERTY(EditAnywhere, Category = "Trigger")
    bool bOneShot = true;

    // 플레이어만 반응
    UPROPERTY(EditAnywhere, Category = "Trigger")
    bool bOnlyPlayer = true;

    // 발동 후 비활성화/파괴
    UPROPERTY(EditAnywhere, Category = "Trigger")
    bool bDestroyAfterTrigger = true;

    // 내부 상태
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