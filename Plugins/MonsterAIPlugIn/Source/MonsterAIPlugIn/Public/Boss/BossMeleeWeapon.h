#pragma once

#include "CoreMinimal.h"
#include "Weapon/MonsterBaseWeapon.h"
#include "GameplayTagContainer.h"  
#include "MonsterTags.h"
#include "BossMeleeWeapon.generated.h"

class UBoxComponent;
class UGameplayEffect;                  
class AAIController;                    
class UBlackboardComponent;             
UCLASS()
class MONSTERAIPLUGIN_API ABossMeleeWeapon : public AMonsterBaseWeapon
{
	GENERATED_BODY()
public:
	ABossMeleeWeapon();

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FVector BoxExtent = FVector(15.f, 15.f, 15.f);

protected:
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* Hitbox = nullptr;
	
// ===== Grab 전용 설정 =====
// 오프너 타이밍에만 트리거 (애니 노티파이로 on/off)
// 예: 오프너 유효 프레임 구간에만 true
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grab|Window")
	bool bGrabOpenerWindow = false;          // [ADD]

	// 같은 창에서 여러 번 발동 방지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grab|Window")
	bool bTriggeredThisWindow = false;       // [ADD]

	// BB 키들 (AI가 쓰는 타겟/확정 신호)
	UPROPERTY(EditAnywhere, Category = "Grab|Blackboard")
	FName BB_TargetActorKey = TEXT("TargetActor");     // [ADD]

	UPROPERTY(EditAnywhere, Category = "Grab|Blackboard")
	FName BB_GrabConfirmedKey = TEXT("bGrabConfirmed");// [ADD]

	// 보스 GA를 깨우는 이벤트 태그 (보스 GA_Boss_Grab가 이걸 Listen)
	UPROPERTY(EditAnywhere, Category = "Grab|Event")
	FGameplayTag BossGrabTriggerTag = MonsterTags::Event_Boss_Grab_Trigger;

	// 피해자(플레이어)에게도 “특수공격 히트” 신호를 보낼지
	UPROPERTY(EditAnywhere, Category = "Grab|Event")
	bool bNotifyVictimOnSpecialHit = true;

	// 피해자에게 보낼 이벤트 태그 (플레이어 GA가 이 태그로 Listen)
	UPROPERTY(EditAnywhere, Category = "Grab|Event")
	FGameplayTag VictimSpecialHitTag = MonsterTags::Event_Player_Grab_Trigger;


protected:

	// 파생에서 ‘그랩 트리거’를 꽂아넣기 위해 ApplyHit 확장
	virtual void ApplyHit(AActor* Victim, const FHitResult& Hit) override;  // [ADD]

	// 특수 공격을 막는 패턴
	UPROPERTY(EditAnywhere, Category = "Grab|Filter")
	FGameplayTagContainer BlockVictimTags; 

public:
	// 애니 노티파이(또는 ANS)에서 창 on/off
	UFUNCTION(BlueprintCallable, Category = "Grab|Window")
	void SetGrabOpenerWindow(bool bOpen);             // [ADD]

	UFUNCTION(BlueprintCallable, Category = "Grab|Window")
	void ResetGrabWindowTrigger();                    // [ADD]
};
